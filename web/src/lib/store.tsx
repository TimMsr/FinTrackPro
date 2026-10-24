import { createContext, useCallback, useContext, useEffect, useMemo, useState, type ReactNode } from 'react'
import { monthKeys } from './analytics.ts'
import { connect, type FinanceApi, type Mode } from './api.ts'
import type { Dataset } from './types.ts'

interface Toast {
  id: number
  kind: 'success' | 'error'
  text: string
}

interface FinanceState {
  data: Dataset
  mode: Mode
  months: string[]
  month: string
  setMonth: (m: string) => void
  // run a mutation, reload, and report the outcome; resolves false on failure
  run: (action: (api: FinanceApi) => Promise<void>, success: string) => Promise<boolean>
  toasts: Toast[]
}

const FinanceContext = createContext<FinanceState | null>(null)

export function FinanceProvider({ children, fallback }: { children: ReactNode; fallback: ReactNode }) {
  const [api, setApi] = useState<FinanceApi | null>(null)
  const [mode, setMode] = useState<Mode>('demo')
  const [data, setData] = useState<Dataset | null>(null)
  const [month, setMonth] = useState('')
  const [toasts, setToasts] = useState<Toast[]>([])

  useEffect(() => {
    connect().then(async ({ api, mode }) => {
      setApi(api)
      setMode(mode)
      const loaded = await api.load()
      setData(loaded)
      setMonth(monthKeys(loaded.transactions).at(-1) ?? '')
    })
  }, [])

  const toast = useCallback((kind: Toast['kind'], text: string) => {
    const id = Date.now() + Math.random()
    setToasts((t) => [...t, { id, kind, text }])
    setTimeout(() => setToasts((t) => t.filter((x) => x.id !== id)), 3500)
  }, [])

  const run = useCallback(
    async (action: (api: FinanceApi) => Promise<void>, success: string) => {
      if (!api) return false
      try {
        await action(api)
        setData(await api.load())
        toast('success', success)
        return true
      } catch (err) {
        toast('error', err instanceof Error ? err.message : 'Something went wrong')
        return false
      }
    },
    [api, toast],
  )

  const value = useMemo(() => {
    if (!data) return null
    const months = monthKeys(data.transactions)
    return { data, mode, months, month: month || months.at(-1) || '', setMonth, run, toasts }
  }, [data, mode, month, run, toasts])

  if (!value) return <>{fallback}</>
  return <FinanceContext.Provider value={value}>{children}</FinanceContext.Provider>
}

export function useFinance() {
  const ctx = useContext(FinanceContext)
  if (!ctx) throw new Error('useFinance must be used inside FinanceProvider')
  return ctx
}
