// Pure functions that turn the raw ledger into the figures the dashboard shows.
// Transfers move money between your own accounts, so they never count as income or spending.

import { EXPENSE_CATEGORIES, type Account, type BudgetLimit, type Category, type Transaction } from './types.ts'

export interface MonthFlow {
  month: string
  income: number
  expenses: number
  net: number
  savingsRate: number
}

const monthOf = (t: Transaction) => t.date.slice(0, 7)
const isFlow = (t: Transaction) => t.category !== 'Transfer'

export function monthKeys(txs: Transaction[]): string[] {
  return [...new Set(txs.map(monthOf))].sort()
}

export function monthlyFlows(txs: Transaction[]): MonthFlow[] {
  const byMonth = new Map<string, MonthFlow>()
  for (const month of monthKeys(txs)) byMonth.set(month, { month, income: 0, expenses: 0, net: 0, savingsRate: 0 })
  for (const t of txs) {
    if (!isFlow(t)) continue
    const row = byMonth.get(monthOf(t))!
    if (t.amount > 0) row.income += t.amount
    else row.expenses -= t.amount
  }
  for (const row of byMonth.values()) {
    row.net = row.income - row.expenses
    row.savingsRate = row.income > 0 ? row.net / row.income : 0
  }
  return [...byMonth.values()]
}

// balance of every account at each month end, plus the total
export function netWorthSeries(txs: Transaction[], accounts: Account[]) {
  const running: Record<string, number> = Object.fromEntries(accounts.map((a) => [a.name, 0]))
  const sorted = [...txs].sort((a, b) => a.date.localeCompare(b.date))
  const series: { month: string; netWorth: number; assets: number; liabilities: number }[] = []
  let i = 0
  for (const month of monthKeys(txs)) {
    while (i < sorted.length && monthOf(sorted[i]) <= month) {
      running[sorted[i].account] = (running[sorted[i].account] ?? 0) + sorted[i].amount
      i++
    }
    let assets = 0
    let liabilities = 0
    for (const value of Object.values(running)) {
      if (value >= 0) assets += value
      else liabilities -= value
    }
    series.push({ month, netWorth: assets - liabilities, assets, liabilities })
  }
  return series
}

export function spendingByCategory(txs: Transaction[], month: string) {
  const totals = new Map<Category, number>(EXPENSE_CATEGORIES.map((c) => [c, 0]))
  for (const t of txs) {
    if (monthOf(t) === month && isFlow(t) && t.amount < 0 && t.category !== 'Income') {
      totals.set(t.category, (totals.get(t.category) ?? 0) - t.amount)
    }
  }
  return [...totals.entries()].map(([category, amount]) => ({ category, amount })).sort((a, b) => b.amount - a.amount)
}

export type BudgetState = 'good' | 'warning' | 'critical'

export function budgetStatus(txs: Transaction[], budgets: BudgetLimit[], month: string) {
  const spent = new Map(spendingByCategory(txs, month).map((s) => [s.category, s.amount]))
  return EXPENSE_CATEGORIES.map((category) => {
    const limit = budgets.find((b) => b.category === category)?.limit ?? 0
    const used = spent.get(category) ?? 0
    const ratio = limit > 0 ? used / limit : used > 0 ? Infinity : 0
    // spending exactly at the limit (e.g. fixed rent) is on plan, not a warning
    const state: BudgetState = ratio > 1 ? 'critical' : ratio >= 0.85 && ratio < 1 ? 'warning' : 'good'
    return { category, limit, spent: used, remaining: limit - used, ratio, state }
  })
}

// this month's spend per category vs the average of the three months before it
export function categoryTrend(txs: Transaction[], month: string) {
  const months = monthKeys(txs)
  const idx = months.indexOf(month)
  const prior = months.slice(Math.max(0, idx - 3), idx)
  const current = new Map(spendingByCategory(txs, month).map((s) => [s.category, s.amount]))
  return EXPENSE_CATEGORIES.map((category) => {
    const avg = prior.length
      ? prior.reduce((sum, m) => sum + (spendingByCategory(txs, m).find((s) => s.category === category)?.amount ?? 0), 0) / prior.length
      : 0
    const now = current.get(category) ?? 0
    return { category, current: now, average: avg, change: avg > 0 ? (now - avg) / avg : 0 }
  })
}

export function average(values: number[]) {
  return values.length ? values.reduce((a, b) => a + b, 0) / values.length : 0
}

// months of spending covered by cash and savings
export function emergencyRunway(accounts: Account[], flows: MonthFlow[]) {
  const liquid = accounts.filter((a) => a.type !== 'credit').reduce((sum, a) => sum + a.balance, 0)
  const burn = average(flows.slice(-4, -1).map((f) => f.expenses)) || average(flows.map((f) => f.expenses))
  return { liquid, burn, months: burn > 0 ? liquid / burn : 0 }
}

// future value of a starting balance plus monthly contributions, compounded monthly
export function projectSavings(start: number, monthly: number, annualRate: number, years: number) {
  const r = annualRate / 12
  const points = [{ year: 0, contributions: start, growth: 0, total: start }]
  let balance = start
  let contributed = start
  for (let m = 1; m <= years * 12; m++) {
    balance = balance * (1 + r) + monthly
    contributed += monthly
    if (m % 12 === 0) points.push({ year: m / 12, contributions: contributed, growth: balance - contributed, total: balance })
  }
  return points
}

export function recomputeBalances(accounts: Account[], txs: Transaction[]): Account[] {
  return accounts.map((a) => {
    const own = txs.filter((t) => t.account === a.name)
    return { ...a, balance: own.reduce((s, t) => s + t.amount, 0), transactionCount: own.length }
  })
}
