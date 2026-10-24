import { ArrowRight, CreditCard, Landmark, PiggyBank, Plus, TrendingUp, Wallet } from 'lucide-react'
import { useMemo, useState } from 'react'
import { CashFlowChart, CategoryBars, NetWorthChart } from '../components/charts.tsx'
import { AccountForm } from '../components/forms.tsx'
import { Card, CategoryTag, Delta, StatCard } from '../components/ui.tsx'
import { monthlyFlows, netWorthSeries, spendingByCategory } from '../lib/analytics.ts'
import { dateLabel, money, monthLabel, percent, signedMoney, todayIso } from '../lib/format.ts'
import { useFinance } from '../lib/store.tsx'
import type { AccountType } from '../lib/types.ts'

const ACCOUNT_ICON: Record<AccountType, typeof Wallet> = { checking: Wallet, savings: PiggyBank, credit: CreditCard }

const change = (now: number, before?: number) => (before ? (now - before) / Math.abs(before) : 0)

export function Overview({ navigate }: { navigate: (page: string) => void }) {
  const { data, month } = useFinance()
  const [addingAccount, setAddingAccount] = useState(false)

  const flows = useMemo(() => monthlyFlows(data.transactions), [data.transactions])
  const worth = useMemo(() => netWorthSeries(data.transactions, data.accounts), [data])
  const spending = useMemo(() => spendingByCategory(data.transactions, month), [data.transactions, month])

  const i = flows.findIndex((f) => f.month === month)
  const cur = flows[i]
  const prev = flows[i - 1]
  const nw = worth[i]
  const nwPrev = worth[i - 1]
  const recent = [...data.transactions].filter((t) => t.date.slice(0, 7) <= month).sort((a, b) => b.date.localeCompare(a.date)).slice(0, 6)
  const rateDiff = cur && prev ? (cur.savingsRate - prev.savingsRate) * 100 : 0

  if (!cur) return <p className="text-ink-3">No transactions yet. Add one to get started.</p>

  return (
    <div className="flex flex-col gap-5">
      <div className="grid grid-cols-1 gap-4 sm:grid-cols-2 xl:grid-cols-4">
        <StatCard label="Net worth" icon={<Landmark size={16} />} value={money(nw.netWorth)} footer={<Delta value={change(nw.netWorth, nwPrev?.netWorth)} />} />
        <StatCard label="Income" icon={<TrendingUp size={16} />} value={money(cur.income)} footer={<Delta value={change(cur.income, prev?.income)} />} />
        <StatCard label="Spending" icon={<Wallet size={16} />} value={money(cur.expenses)} footer={<Delta value={change(cur.expenses, prev?.expenses)} invert />} />
        <StatCard
          label="Savings rate"
          icon={<PiggyBank size={16} />}
          value={percent(cur.savingsRate)}
          footer={
            <span className="text-[13px] text-ink-3">
              <span className={`num font-medium ${rateDiff >= 0 ? 'text-positive' : 'text-negative'}`}>
                {rateDiff >= 0 ? '▲' : '▼'} {Math.abs(rateDiff).toFixed(1)} pts
              </span>{' '}
              vs last month
            </span>
          }
        />
      </div>

      <div className="grid grid-cols-1 gap-5 lg:grid-cols-3">
        <Card title="Net worth" subtitle="Month-end balance across all accounts" className="lg:col-span-2">
          <NetWorthChart data={worth.slice(0, i + 1)} />
        </Card>
        <Card
          title="Accounts"
          subtitle={`${data.accounts.length} linked`}
          action={
            <button onClick={() => setAddingAccount(true)} className="rounded-md p-1.5 text-ink-3 hover:bg-surface-2 hover:text-ink" aria-label="Add account">
              <Plus size={16} />
            </button>
          }
        >
          <ul className="flex flex-col divide-y divide-line">
            {data.accounts.map((a) => {
              const Icon = ACCOUNT_ICON[a.type]
              return (
                <li key={a.name} className="flex items-center gap-3 py-3 first:pt-0 last:pb-0">
                  <span className="grid size-9 place-items-center rounded-lg bg-accent-soft text-accent">
                    <Icon size={17} />
                  </span>
                  <div className="min-w-0 flex-1">
                    <div className="truncate text-[14px] font-medium">{a.name}</div>
                    <div className="text-[12px] capitalize text-ink-3">
                      {a.type}
                      {a.interestRate ? ` · ${percent(a.interestRate, 2)} APY` : ''}
                    </div>
                  </div>
                  <div className={`num text-[14px] font-medium ${a.balance < 0 ? 'text-negative' : ''}`}>{money(a.balance)}</div>
                </li>
              )
            })}
          </ul>
        </Card>
      </div>

      <div className="grid grid-cols-1 gap-5 lg:grid-cols-3">
        <Card title="Cash flow" subtitle="Income vs spending, transfers excluded" className="lg:col-span-2">
          <CashFlowChart data={flows.slice(0, i + 1)} />
        </Card>
        <Card title="Spending by category" subtitle={`${monthLabel(month, 'long')}${todayIso().startsWith(month) ? ' · month to date' : ''}`}>
          <CategoryBars data={spending} />
        </Card>
      </div>

      <Card
        title="Recent activity"
        action={
          <button onClick={() => navigate('transactions')} className="flex items-center gap-1 text-[13px] font-medium text-accent hover:underline">
            View all <ArrowRight size={14} />
          </button>
        }
      >
        <ul className="flex flex-col divide-y divide-line">
          {recent.map((t) => (
            <li key={t.account + t.id} className="flex items-center gap-3 py-2.5 first:pt-0 last:pb-0">
              <div className="min-w-0 flex-1">
                <div className="truncate text-[14px] font-medium">{t.note || t.category}</div>
                <div className="text-[12px] text-ink-3">{dateLabel(t.date)} · {t.account}</div>
              </div>
              <span className="hidden sm:inline"><CategoryTag category={t.category} /></span>
              <div className={`num w-28 text-right text-[14px] font-medium ${t.amount > 0 ? 'text-positive' : ''}`}>{signedMoney(t.amount)}</div>
            </li>
          ))}
        </ul>
      </Card>

      {addingAccount && <AccountForm onClose={() => setAddingAccount(false)} />}
    </div>
  )
}
