// Data access. LiveApi talks to the C++ server; DemoApi runs the same operations in the browser
// on generated sample data so the dashboard can be hosted statically.

import { recomputeBalances } from './analytics.ts'
import { generateSample } from './sampleData.ts'
import type { AccountType, Category, Dataset, NewTransaction, NewTransfer } from './types.ts'

export interface FinanceApi {
  load(): Promise<Dataset>
  addTransaction(tx: NewTransaction): Promise<void>
  transfer(t: NewTransfer): Promise<void>
  setBudget(category: Category, limit: number): Promise<void>
  addAccount(name: string, type: AccountType): Promise<void>
}

export type Mode = 'live' | 'demo'

async function request<T>(path: string, init?: RequestInit): Promise<T> {
  const res = await fetch(path, { headers: { 'Content-Type': 'application/json' }, ...init })
  const body = await res.json().catch(() => ({}))
  if (!res.ok) throw new Error(body.error ?? `Request failed (${res.status})`)
  return body as T
}

class LiveApi implements FinanceApi {
  async load(): Promise<Dataset> {
    const [accounts, transactions, budgets] = await Promise.all([
      request<Dataset['accounts']>('/api/accounts'),
      request<Dataset['transactions']>('/api/transactions'),
      request<Dataset['budgets']>('/api/budgets'),
    ])
    return { accounts, transactions, budgets }
  }
  async addTransaction(tx: NewTransaction) {
    await request('/api/transactions', { method: 'POST', body: JSON.stringify(tx) })
  }
  async transfer(t: NewTransfer) {
    await request('/api/transfers', { method: 'POST', body: JSON.stringify(t) })
  }
  async setBudget(category: Category, limit: number) {
    await request('/api/budgets', { method: 'PUT', body: JSON.stringify({ category, limit }) })
  }
  async addAccount(name: string, type: AccountType) {
    await request('/api/accounts', { method: 'POST', body: JSON.stringify({ name, type }) })
  }
}

const STORAGE_KEY = 'fintrack-demo-v1'

class DemoApi implements FinanceApi {
  private data: Dataset

  constructor() {
    let saved: Dataset | null = null
    try {
      const raw = localStorage.getItem(STORAGE_KEY)
      if (raw) saved = JSON.parse(raw)
    } catch {
      saved = null
    }
    this.data = saved ?? generateSample()
  }

  private commit() {
    this.data.accounts = recomputeBalances(this.data.accounts, this.data.transactions)
    try {
      localStorage.setItem(STORAGE_KEY, JSON.stringify(this.data))
    } catch {
      // storage unavailable: changes simply last for this session
    }
  }

  private requireAccount(name: string) {
    const account = this.data.accounts.find((a) => a.name === name)
    if (!account) throw new Error(`Unknown account: ${name}`)
    return account
  }

  async load() {
    return structuredClone(this.data)
  }
  async addTransaction(tx: NewTransaction) {
    this.requireAccount(tx.account)
    this.data.transactions.push({ ...tx, id: `T${Date.now()}` })
    this.commit()
  }
  async transfer({ from, to, date, amount }: NewTransfer) {
    const source = this.requireAccount(from)
    const dest = this.requireAccount(to)
    if (from === to) throw new Error('Choose two different accounts')
    if (source.type === 'credit' && dest.type === 'savings') throw new Error('Transfers from a credit card into savings are not allowed')
    const id = `T${Date.now()}`
    this.data.transactions.push(
      { id: `${id}-OUT`, date, account: from, category: 'Transfer', amount: -amount, note: `Transfer to ${to}` },
      { id: `${id}-IN`, date, account: to, category: 'Transfer', amount, note: `Transfer from ${from}` },
    )
    this.commit()
  }
  async setBudget(category: Category, limit: number) {
    const existing = this.data.budgets.find((b) => b.category === category)
    if (existing) existing.limit = limit
    else this.data.budgets.push({ category, limit })
    this.commit()
  }
  async addAccount(name: string, type: AccountType) {
    if (this.data.accounts.some((a) => a.name === name)) throw new Error('An account with that name already exists')
    this.data.accounts.push({ name, type, balance: 0, transactionCount: 0, ...(type === 'savings' ? { interestRate: 0.05 } : {}) })
    this.commit()
  }
}

export function resetDemo() {
  try {
    localStorage.removeItem(STORAGE_KEY)
  } catch {
    // nothing stored
  }
}

// use the C++ backend when it answers, otherwise fall back to the in-browser demo
export async function connect(): Promise<{ api: FinanceApi; mode: Mode }> {
  if (import.meta.env.MODE !== 'demo') {
    try {
      const controller = new AbortController()
      const timer = setTimeout(() => controller.abort(), 1500)
      const res = await fetch('/api/health', { signal: controller.signal })
      clearTimeout(timer)
      if (res.ok) return { api: new LiveApi(), mode: 'live' }
    } catch {
      // backend not running
    }
  }
  return { api: new DemoApi(), mode: 'demo' }
}
