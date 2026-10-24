// Shapes returned by the C++ API (amounts are integer cents, dates are YYYY-MM-DD)

export type AccountType = 'checking' | 'savings' | 'credit'

export type Category = 'Food' | 'Rent' | 'Utilities' | 'Transport' | 'Income' | 'Transfer' | 'Other'

export const CATEGORIES: Category[] = ['Food', 'Rent', 'Utilities', 'Transport', 'Income', 'Transfer', 'Other']
export const EXPENSE_CATEGORIES: Category[] = ['Rent', 'Food', 'Utilities', 'Transport', 'Other']

export interface Account {
  name: string
  type: AccountType
  balance: number
  transactionCount: number
  interestRate?: number
}

export interface Transaction {
  id: string
  date: string
  account: string
  category: Category
  amount: number
  note: string
}

export interface BudgetLimit {
  category: Category
  limit: number
}

export interface Dataset {
  accounts: Account[]
  transactions: Transaction[]
  budgets: BudgetLimit[]
}

export interface NewTransaction {
  account: string
  date: string
  amount: number
  category: Category
  note: string
}

export interface NewTransfer {
  from: string
  to: string
  date: string
  amount: number
}
