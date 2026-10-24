import { ArrowLeftRight, BarChart3, LayoutDashboard, ListOrdered, Moon, Plus, RotateCcw, Sun, Target } from 'lucide-react'
import { useEffect, useState } from 'react'
import { TransactionForm, TransferForm } from './components/forms.tsx'
import { Button, Select } from './components/ui.tsx'
import { resetDemo } from './lib/api.ts'
import { monthLabel } from './lib/format.ts'
import { useFinance } from './lib/store.tsx'
import { Budgets } from './pages/Budgets.tsx'
import { Insights } from './pages/Insights.tsx'
import { Overview } from './pages/Overview.tsx'
import { Transactions } from './pages/Transactions.tsx'

const PAGES = [
  { id: 'overview', label: 'Overview', icon: LayoutDashboard, subtitle: 'Your financial position at a glance' },
  { id: 'transactions', label: 'Transactions', icon: ListOrdered, subtitle: 'Every entry across your accounts' },
  { id: 'budgets', label: 'Budgets', icon: Target, subtitle: 'Spending limits by category' },
  { id: 'insights', label: 'Insights', icon: BarChart3, subtitle: 'Trends, resilience and long-term projections' },
] as const

type PageId = (typeof PAGES)[number]['id']

const pageFromHash = (): PageId => {
  const id = window.location.hash.replace('#/', '')
  return PAGES.some((p) => p.id === id) ? (id as PageId) : 'overview'
}

function useTheme() {
  const [dark, setDark] = useState(() => {
    const set = document.documentElement.dataset.theme
    return set ? set === 'dark' : window.matchMedia('(prefers-color-scheme: dark)').matches
  })
  const toggle = () => {
    const next = dark ? 'light' : 'dark'
    document.documentElement.dataset.theme = next
    try {
      localStorage.setItem('fintrack-theme', next)
    } catch {
      // preference just won't persist
    }
    setDark(!dark)
  }
  return { dark, toggle }
}

function Logo() {
  return (
    <div className="flex items-center gap-2.5">
      <span className="grid size-8 place-items-center rounded-lg bg-accent text-white">
        <svg viewBox="0 0 24 24" className="size-[18px]" fill="none" stroke="currentColor" strokeWidth={2.5} strokeLinecap="round" strokeLinejoin="round">
          <path d="M4 16l5-5 4 3 7-8" />
        </svg>
      </span>
      <span className="text-[15px] font-semibold tracking-tight">FinTrack Pro</span>
    </div>
  )
}

export default function App() {
  const { mode, months, month, setMonth, toasts } = useFinance()
  const [page, setPage] = useState<PageId>(pageFromHash)
  const [modal, setModal] = useState<'tx' | 'transfer' | null>(null)
  const { dark, toggle } = useTheme()

  useEffect(() => {
    const onHash = () => setPage(pageFromHash())
    window.addEventListener('hashchange', onHash)
    return () => window.removeEventListener('hashchange', onHash)
  }, [])

  const navigate = (id: string) => {
    window.location.hash = `#/${id}`
    window.scrollTo({ top: 0 })
  }
  const current = PAGES.find((p) => p.id === page)!

  return (
    <div className="min-h-screen lg:grid lg:grid-cols-[232px_1fr]">
      <aside className="sticky top-0 hidden h-screen flex-col border-r border-line bg-surface px-4 py-5 lg:flex">
        <div className="px-2"><Logo /></div>
        <nav className="mt-8 flex flex-col gap-0.5">
          {PAGES.map((p) => (
            <button
              key={p.id}
              onClick={() => navigate(p.id)}
              className={`flex items-center gap-2.5 rounded-lg px-2.5 py-2 text-[14px] font-medium transition ${page === p.id ? 'bg-accent-soft text-accent' : 'text-ink-2 hover:bg-surface-2 hover:text-ink'}`}
            >
              <p.icon size={17} />
              {p.label}
            </button>
          ))}
        </nav>
        <div className="mt-auto flex flex-col gap-3">
          <div className="rounded-lg border border-line bg-surface-2 p-3 text-[12px] leading-relaxed text-ink-3">
            <div className="mb-1 flex items-center gap-1.5 font-medium text-ink-2">
              <span className={`size-1.5 rounded-full ${mode === 'live' ? 'bg-good' : 'bg-warning'}`} />
              {mode === 'live' ? 'Connected to C++ engine' : 'Demo mode'}
            </div>
            {mode === 'live' ? 'Changes are written to your CSV ledgers.' : 'Sample data stored in this browser.'}
            {mode === 'demo' && (
              <button
                onClick={() => {
                  resetDemo()
                  window.location.reload()
                }}
                className="mt-2 flex items-center gap-1 font-medium text-accent hover:underline"
              >
                <RotateCcw size={12} /> Reset sample data
              </button>
            )}
          </div>
          <Button variant="ghost" onClick={toggle} className="justify-start">
            {dark ? <Sun size={16} /> : <Moon size={16} />} {dark ? 'Light mode' : 'Dark mode'}
          </Button>
        </div>
      </aside>

      <div className="min-w-0">
        {/* mobile header */}
        <div className="sticky top-0 z-30 border-b border-line bg-surface/90 backdrop-blur lg:hidden">
          <div className="flex items-center justify-between px-4 py-3">
            <Logo />
            <button onClick={toggle} className="rounded-md p-2 text-ink-2 hover:bg-surface-2" aria-label="Toggle theme">
              {dark ? <Sun size={17} /> : <Moon size={17} />}
            </button>
          </div>
          <nav className="flex gap-1 overflow-x-auto px-3 pb-2">
            {PAGES.map((p) => (
              <button
                key={p.id}
                onClick={() => navigate(p.id)}
                className={`shrink-0 rounded-lg px-3 py-1.5 text-[13px] font-medium ${page === p.id ? 'bg-accent-soft text-accent' : 'text-ink-2'}`}
              >
                {p.label}
              </button>
            ))}
          </nav>
        </div>

        <main className="mx-auto max-w-[1280px] px-4 py-6 sm:px-8 sm:py-8">
          <header className="mb-6 flex flex-col gap-4 sm:flex-row sm:items-end sm:justify-between">
            <div>
              <h1 className="text-[24px] font-semibold tracking-tight">{current.label}</h1>
              <p className="mt-1 text-[14px] text-ink-3">{current.subtitle}</p>
            </div>
            <div className="flex flex-wrap items-center gap-2">
              {page !== 'transactions' && (
                <Select value={month} onChange={(e) => setMonth(e.target.value)} className="w-44!" aria-label="Month">
                  {[...months].reverse().map((m) => <option key={m} value={m}>{monthLabel(m, 'long')}</option>)}
                </Select>
              )}
              <Button variant="secondary" onClick={() => setModal('transfer')}><ArrowLeftRight size={15} /> Transfer</Button>
              <Button onClick={() => setModal('tx')}><Plus size={15} /> Add transaction</Button>
            </div>
          </header>

          {page === 'overview' && <Overview navigate={navigate} />}
          {page === 'transactions' && <Transactions />}
          {page === 'budgets' && <Budgets />}
          {page === 'insights' && <Insights />}

          <footer className="mt-10 text-center text-[12px] text-ink-3">
            FinTrack Pro · React dashboard on a C++17 ledger engine
          </footer>
        </main>
      </div>

      {modal === 'tx' && <TransactionForm onClose={() => setModal(null)} />}
      {modal === 'transfer' && <TransferForm onClose={() => setModal(null)} />}

      <div className="pointer-events-none fixed right-4 bottom-4 z-[60] flex flex-col gap-2" aria-live="polite">
        {toasts.map((t) => (
          <div key={t.id} className="card pointer-events-auto flex items-center gap-2 px-4 py-3 text-[13px] font-medium shadow-lg">
            <span className={`size-2 rounded-full ${t.kind === 'success' ? 'bg-good' : 'bg-critical'}`} />
            {t.text}
          </div>
        ))}
      </div>
    </div>
  )
}
