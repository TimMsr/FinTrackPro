// Deterministic sample data: twelve months of a young professional's finances.
// Used by the in-browser demo and by scripts/generate-sample.ts to seed the C++ backend.

import type { Account, AccountType, BudgetLimit, Category, Dataset, Transaction } from './types.ts'

export const CHECKING = 'Everyday Checking'
export const SAVINGS = 'High-Yield Savings'
export const CARD = 'Rewards Card'

export const SAMPLE_ACCOUNTS: { name: string; type: AccountType; interestRate?: number }[] = [
  { name: CHECKING, type: 'checking' },
  { name: SAVINGS, type: 'savings', interestRate: 0.05 },
  { name: CARD, type: 'credit' },
]

export const SAMPLE_BUDGETS: BudgetLimit[] = [
  { category: 'Rent', limit: 195000 },
  { category: 'Food', limit: 65000 },
  { category: 'Utilities', limit: 26000 },
  { category: 'Transport', limit: 26000 },
  { category: 'Other', limit: 40000 },
]

// small seeded PRNG so every run produces the same history
function mulberry32(seed: number) {
  return () => {
    seed |= 0
    seed = (seed + 0x6d2b79f5) | 0
    let t = Math.imul(seed ^ (seed >>> 15), 1 | seed)
    t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296
  }
}

const pad = (n: number) => String(n).padStart(2, '0')

export function generateSample(today = new Date(), months = 12): Dataset {
  const rand = mulberry32(2026)
  const between = (lo: number, hi: number) => Math.round((lo + rand() * (hi - lo)) * 100)
  const pick = <T,>(items: T[]) => items[Math.floor(rand() * items.length)]

  const todayKey = `${today.getFullYear()}-${pad(today.getMonth() + 1)}-${pad(today.getDate())}`
  const txs: Transaction[] = []
  const counters: Record<string, number> = {}
  const prefix: Record<string, string> = { [CHECKING]: 'CHK', [SAVINGS]: 'SAV', [CARD]: 'CRD' }
  const balances: Record<string, number> = { [CHECKING]: 0, [SAVINGS]: 0, [CARD]: 0 }

  const add = (account: string, date: string, amount: number, category: Category, note: string) => {
    if (date > todayKey) return
    counters[account] = (counters[account] ?? 0) + 1
    txs.push({ id: `${prefix[account]}-${String(counters[account]).padStart(4, '0')}`, date, account, category, amount, note })
    balances[account] += amount
  }
  const transfer = (from: string, to: string, date: string, amount: number) => {
    add(from, date, -amount, 'Transfer', `Transfer to ${to}`)
    add(to, date, amount, 'Transfer', `Transfer from ${from}`)
  }

  const groceries = ['Groceries · Green Market', 'Groceries · Corner Co-op', 'Groceries · FreshWay', 'Farmers market']
  const dining = ['Dinner · Osteria Luca', 'Coffee · Blue Door', 'Lunch · Sushi Bar', 'Brunch · Hearth Café', 'Takeout · Thai Garden', 'Drinks with friends']
  const shopping = ['Bookstore', 'Clothing', 'Home goods', 'Electronics', 'Pharmacy', 'Concert tickets']

  const start = new Date(today.getFullYear(), today.getMonth() - (months - 1), 1)
  let cardChargesLastMonth = 0

  for (let i = 0; i < months; i++) {
    const d = new Date(start.getFullYear(), start.getMonth() + i, 1)
    const month = `${d.getFullYear()}-${pad(d.getMonth() + 1)}`
    const calMonth = d.getMonth() + 1
    const lastDay = new Date(d.getFullYear(), d.getMonth() + 1, 0).getDate()
    const day = (n: number) => `${month}-${pad(Math.min(n, lastDay))}`
    const randomDay = () => day(1 + Math.floor(rand() * lastDay))
    let cardCharges = 0
    const charge = (date: string, amount: number, category: Category, note: string) => {
      if (date <= todayKey) cardCharges += amount
      add(CARD, date, -amount, category, note)
    }

    if (i === 0) {
      add(CHECKING, day(1), 420000, 'Transfer', 'Opening balance')
      add(SAVINGS, day(1), 1450000, 'Transfer', 'Opening balance')
    }

    // income
    add(CHECKING, day(1), 265000, 'Income', 'Salary · Northwind Analytics')
    add(CHECKING, day(15), 265000, 'Income', 'Salary · Northwind Analytics')
    if (calMonth === 3) add(CHECKING, day(15), 400000, 'Income', 'Annual performance bonus')
    if (calMonth === 4) add(CHECKING, day(22), 86000, 'Income', 'Tax refund')

    // fixed costs from checking
    add(CHECKING, day(1), -195000, 'Rent', 'Rent · Maple Court Apartments')
    const winter = [12, 1, 2].includes(calMonth) ? 1.6 : [6, 7, 8].includes(calMonth) ? 1.35 : 1
    add(CHECKING, day(12), -Math.round(between(62, 88) * winter), 'Utilities', 'Electricity · City Power')
    add(CHECKING, day(18), -6500, 'Utilities', 'Internet · Fibernet')

    // savings habit: pay yourself first, plus most of the bonus
    transfer(CHECKING, SAVINGS, day(2), 120000)
    if (calMonth === 3) transfer(CHECKING, SAVINGS, day(16), 300000)

    // card spending
    charge(day(3), 13200, 'Transport', 'Monthly transit pass')
    charge(day(20), 4500, 'Utilities', 'Mobile plan')
    charge(day(6), 4900, 'Other', 'Gym membership')
    charge(day(9), 1599, 'Other', 'Streaming subscription')
    for (let k = 0, n = 5 + Math.floor(rand() * 3); k < n; k++) charge(randomDay(), between(38, 118), 'Food', pick(groceries))
    for (let k = 0, n = 4 + Math.floor(rand() * 4); k < n; k++) charge(randomDay(), between(12, 64), 'Food', pick(dining))
    for (let k = 0, n = 2 + Math.floor(rand() * 3); k < n; k++) charge(randomDay(), between(11, 36), 'Transport', 'Rideshare')
    for (let k = 0, n = 1 + Math.floor(rand() * 3); k < n; k++) charge(randomDay(), between(20, 160), 'Other', pick(shopping))
    if (calMonth === 12) charge(day(14), 46000, 'Other', 'Holiday gifts')
    if (calMonth === 7) charge(day(10), 68000, 'Transport', 'Flights · summer trip')

    // statement for last month's card spending is paid in full
    if (cardChargesLastMonth > 0) transfer(CHECKING, CARD, day(26), cardChargesLastMonth)
    cardChargesLastMonth = cardCharges

    // savings interest at 1/12 of the APY on the month-end balance
    add(SAVINGS, day(lastDay), Math.round(balances[SAVINGS] * (0.05 / 12)), 'Income', 'Monthly interest')
  }

  txs.sort((a, b) => a.date.localeCompare(b.date) || a.id.localeCompare(b.id))

  const accounts: Account[] = SAMPLE_ACCOUNTS.map((a) => ({
    ...a,
    balance: balances[a.name],
    transactionCount: txs.filter((t) => t.account === a.name).length,
  }))
  return { accounts, transactions: txs, budgets: SAMPLE_BUDGETS.map((b) => ({ ...b })) }
}
