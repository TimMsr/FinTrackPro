import { AlertTriangle, CheckCircle2, Pencil, XCircle } from 'lucide-react'
import { useState, type FormEvent } from 'react'
import { Card, CATEGORY_COLOR, Input, StatCard } from '../components/ui.tsx'
import { budgetStatus, type BudgetState } from '../lib/analytics.ts'
import { money, monthLabel, parseCents, todayIso } from '../lib/format.ts'
import { useFinance } from '../lib/store.tsx'
import type { Category } from '../lib/types.ts'

const STATE: Record<BudgetState, { label: string; color: string; Icon: typeof CheckCircle2 }> = {
  good: { label: 'On track', color: 'var(--good)', Icon: CheckCircle2 },
  warning: { label: 'Near limit', color: 'var(--warning)', Icon: AlertTriangle },
  critical: { label: 'Over budget', color: 'var(--critical)', Icon: XCircle },
}

function LimitEditor({ category, limit, onDone }: { category: Category; limit: number; onDone: () => void }) {
  const { run } = useFinance()
  const [value, setValue] = useState((limit / 100).toFixed(0))
  const submit = async (e: FormEvent) => {
    e.preventDefault()
    const cents = parseCents(value)
    if (!Number.isFinite(cents)) return
    if (await run((api) => api.setBudget(category, cents), `${category} budget updated`)) onDone()
  }
  return (
    <form onSubmit={submit} className="flex items-center gap-2">
      <Input value={value} onChange={(e) => setValue(e.target.value)} inputMode="decimal" className="w-28" autoFocus onBlur={onDone} aria-label={`${category} limit`} />
    </form>
  )
}

export function Budgets() {
  const { data, month } = useFinance()
  const [editing, setEditing] = useState<Category | null>(null)
  const rows = budgetStatus(data.transactions, data.budgets, month)

  const totalLimit = rows.reduce((s, r) => s + r.limit, 0)
  const totalSpent = rows.reduce((s, r) => s + r.spent, 0)
  const [y, m] = month.split('-').map(Number)
  const daysInMonth = new Date(y, m, 0).getDate()
  const today = todayIso()
  const isCurrent = today.startsWith(month)
  const daysLeft = isCurrent ? daysInMonth - Number(today.slice(8)) + 1 : 0
  const left = totalLimit - totalSpent

  return (
    <div className="flex flex-col gap-5">
      <div className="grid grid-cols-1 gap-4 sm:grid-cols-3">
        <StatCard label="Monthly budget" value={money(totalLimit)} footer={<span className="text-[13px] text-ink-3">{monthLabel(month, 'long')}</span>} />
        <StatCard label="Spent" value={money(totalSpent)} footer={<span className="text-[13px] text-ink-3">{totalLimit ? Math.round((totalSpent / totalLimit) * 100) : 0}% of budget used</span>} />
        <StatCard
          label={left >= 0 ? 'Left to spend' : 'Over budget by'}
          value={money(Math.abs(left))}
          footer={
            <span className="text-[13px] text-ink-3">
              {isCurrent && left > 0 ? <><span className="num font-medium text-ink">{money(left / daysLeft)}</span> per day for {daysLeft} days</> : isCurrent ? 'Consider trimming discretionary spend' : 'Month closed'}
            </span>
          }
        />
      </div>

      <Card title="Category budgets" subtitle="Click a limit to edit it. Spending is compared against the selected month.">
        <ul className="flex flex-col divide-y divide-line">
          {rows.map((r) => {
            const s = STATE[r.state]
            return (
              <li key={r.category} className="grid grid-cols-1 gap-3 py-4 first:pt-0 last:pb-0 sm:grid-cols-[180px_1fr_auto] sm:items-center sm:gap-6">
                <div className="flex items-center gap-2.5">
                  <span className="size-2.5 rounded-full" style={{ background: CATEGORY_COLOR[r.category] }} />
                  <span className="text-[14px] font-medium">{r.category}</span>
                </div>
                <div>
                  <div className="mb-1.5 flex items-baseline justify-between text-[13px]">
                    <span className="num text-ink-2">
                      <span className="font-medium text-ink">{money(r.spent)}</span> of{' '}
                      {editing === r.category ? null : (
                        <button onClick={() => setEditing(r.category)} className="group inline-flex items-center gap-1 rounded px-1 hover:bg-surface-2">
                          {money(r.limit)} <Pencil size={12} className="text-ink-3 opacity-0 group-hover:opacity-100" />
                        </button>
                      )}
                    </span>
                    {editing === r.category && <LimitEditor category={r.category} limit={r.limit} onDone={() => setEditing(null)} />}
                    <span className="num text-ink-3">{r.remaining >= 0 ? `${money(r.remaining)} left` : `${money(-r.remaining)} over`}</span>
                  </div>
                  <div className="h-2 overflow-hidden rounded-full bg-surface-2" role="progressbar" aria-valuenow={Math.round(r.ratio * 100)} aria-valuemin={0} aria-valuemax={100}>
                    <div className="h-2 rounded-full transition-[width] duration-500" style={{ width: `${Math.min(100, r.ratio * 100)}%`, background: s.color }} />
                  </div>
                </div>
                <span className="inline-flex items-center gap-1.5 text-[13px] font-medium text-ink-2 sm:w-28 sm:justify-end">
                  <s.Icon size={15} style={{ color: s.color }} />
                  {s.label}
                </span>
              </li>
            )
          })}
        </ul>
      </Card>
    </div>
  )
}
