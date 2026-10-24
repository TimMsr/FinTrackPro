import { Area, AreaChart, Bar, BarChart, CartesianGrid, ResponsiveContainer, Tooltip, XAxis, YAxis } from 'recharts'
import type { MonthFlow } from '../lib/analytics.ts'
import { money, moneyCompact, monthLabel, percent } from '../lib/format.ts'
import type { Category } from '../lib/types.ts'
import { CATEGORY_COLOR } from './ui.tsx'

const axis = { stroke: 'var(--ink-3)', fontSize: 12, tickLine: false, axisLine: false } as const

interface TooltipRow {
  label: string
  value: string
  color?: string
}

function TooltipBox({ title, rows }: { title: string; rows: TooltipRow[] }) {
  return (
    <div className="min-w-44 rounded-lg border border-line bg-surface px-3 py-2.5 text-[12px] shadow-lg">
      <div className="mb-1.5 font-medium text-ink">{title}</div>
      {rows.map((r) => (
        <div key={r.label} className="flex items-center justify-between gap-4 py-0.5 text-ink-2">
          <span className="flex items-center gap-1.5">
            {r.color && <span className="size-2 rounded-full" style={{ background: r.color }} />}
            {r.label}
          </span>
          <span className="num font-medium text-ink">{r.value}</span>
        </div>
      ))}
    </div>
  )
}

export function Legend({ items }: { items: { label: string; color: string }[] }) {
  return (
    <div className="flex flex-wrap gap-4 text-[12px] text-ink-2">
      {items.map((i) => (
        <span key={i.label} className="flex items-center gap-1.5">
          <span className="size-2.5 rounded-sm" style={{ background: i.color }} />
          {i.label}
        </span>
      ))}
    </div>
  )
}

export function NetWorthChart({ data }: { data: { month: string; netWorth: number; assets: number; liabilities: number }[] }) {
  return (
    <ResponsiveContainer width="100%" height={260}>
      <AreaChart data={data} margin={{ top: 8, right: 4, bottom: 0, left: 0 }}>
        <defs>
          <linearGradient id="nw-fill" x1="0" y1="0" x2="0" y2="1">
            <stop offset="0%" stopColor="var(--series-1)" stopOpacity={0.22} />
            <stop offset="100%" stopColor="var(--series-1)" stopOpacity={0} />
          </linearGradient>
        </defs>
        <CartesianGrid vertical={false} stroke="var(--grid)" />
        <XAxis dataKey="month" tickFormatter={(m) => monthLabel(m)} {...axis} dy={6} />
        <YAxis tickFormatter={(v) => moneyCompact(v)} {...axis} width={56} domain={['dataMin - 100000', 'auto']} />
        <Tooltip
          cursor={{ stroke: 'var(--ink-3)', strokeDasharray: '3 3' }}
          content={({ active, payload }) => {
            if (!active || !payload?.length) return null
            const p = payload[0].payload as (typeof data)[number]
            return (
              <TooltipBox
                title={monthLabel(p.month, 'long')}
                rows={[
                  { label: 'Net worth', value: money(p.netWorth), color: 'var(--series-1)' },
                  { label: 'Assets', value: money(p.assets) },
                  { label: 'Card balance', value: money(-p.liabilities) },
                ]}
              />
            )
          }}
        />
        <Area
          type="monotone"
          dataKey="netWorth"
          stroke="var(--series-1)"
          strokeWidth={2}
          fill="url(#nw-fill)"
          activeDot={{ r: 5, strokeWidth: 2, stroke: 'var(--surface)' }}
        />
      </AreaChart>
    </ResponsiveContainer>
  )
}

export function CashFlowChart({ data }: { data: MonthFlow[] }) {
  return (
    <div>
      <Legend items={[{ label: 'Income', color: 'var(--series-1)' }, { label: 'Spending', color: 'var(--series-2)' }]} />
      <ResponsiveContainer width="100%" height={240}>
        <BarChart data={data} margin={{ top: 16, right: 4, bottom: 0, left: 0 }} barGap={2} barCategoryGap="22%">
          <CartesianGrid vertical={false} stroke="var(--grid)" />
          <XAxis dataKey="month" tickFormatter={(m) => monthLabel(m)} {...axis} dy={6} />
          <YAxis tickFormatter={(v) => moneyCompact(v)} {...axis} width={56} />
          <Tooltip
            cursor={{ fill: 'var(--surface-2)' }}
            content={({ active, payload }) => {
              if (!active || !payload?.length) return null
              const p = payload[0].payload as MonthFlow
              return (
                <TooltipBox
                  title={monthLabel(p.month, 'long')}
                  rows={[
                    { label: 'Income', value: money(p.income), color: 'var(--series-1)' },
                    { label: 'Spending', value: money(p.expenses), color: 'var(--series-2)' },
                    { label: 'Net saved', value: money(p.net) },
                    { label: 'Savings rate', value: percent(p.savingsRate) },
                  ]}
                />
              )
            }}
          />
          <Bar dataKey="income" fill="var(--series-1)" radius={[4, 4, 0, 0]} maxBarSize={18} />
          <Bar dataKey="expenses" fill="var(--series-2)" radius={[4, 4, 0, 0]} maxBarSize={18} />
        </BarChart>
      </ResponsiveContainer>
    </div>
  )
}

// ranked horizontal bars drawn in HTML so labels never collide
export function CategoryBars({ data }: { data: { category: Category; amount: number }[] }) {
  const total = data.reduce((s, d) => s + d.amount, 0)
  const max = Math.max(1, ...data.map((d) => d.amount))
  return (
    <ul className="flex flex-col gap-3.5">
      {data.map((d) => (
        <li key={d.category} className="group" title={`${d.category}: ${money(d.amount)} (${percent(total ? d.amount / total : 0)})`}>
          <div className="mb-1.5 flex items-baseline justify-between text-[13px]">
            <span className="flex items-center gap-2 font-medium text-ink">
              <span className="size-2 rounded-full" style={{ background: CATEGORY_COLOR[d.category] }} />
              {d.category}
            </span>
            <span className="num text-ink-2">
              {money(d.amount)} <span className="text-ink-3">· {percent(total ? d.amount / total : 0, 0)}</span>
            </span>
          </div>
          <div className="h-2 rounded-full bg-surface-2">
            <div
              className="h-2 rounded-full transition-[width] duration-500 group-hover:brightness-110"
              style={{ width: `${(d.amount / max) * 100}%`, background: CATEGORY_COLOR[d.category] }}
            />
          </div>
        </li>
      ))}
    </ul>
  )
}

export function ProjectionChart({ data }: { data: { year: number; contributions: number; growth: number; total: number }[] }) {
  return (
    <div>
      <Legend items={[{ label: 'Contributions', color: 'var(--series-1)' }, { label: 'Investment growth', color: 'var(--series-3)' }]} />
      <ResponsiveContainer width="100%" height={260}>
        <AreaChart data={data} margin={{ top: 16, right: 4, bottom: 0, left: 0 }}>
          <CartesianGrid vertical={false} stroke="var(--grid)" />
          <XAxis dataKey="year" tickFormatter={(y) => (y === 0 ? 'Now' : `Y${y}`)} {...axis} dy={6} />
          <YAxis tickFormatter={(v) => moneyCompact(v)} {...axis} width={56} />
          <Tooltip
            cursor={{ stroke: 'var(--ink-3)', strokeDasharray: '3 3' }}
            content={({ active, payload }) => {
              if (!active || !payload?.length) return null
              const p = payload[0].payload as (typeof data)[number]
              return (
                <TooltipBox
                  title={p.year === 0 ? 'Today' : `After ${p.year} year${p.year > 1 ? 's' : ''}`}
                  rows={[
                    { label: 'Contributions', value: money(p.contributions), color: 'var(--series-1)' },
                    { label: 'Growth', value: money(p.growth), color: 'var(--series-3)' },
                    { label: 'Total', value: money(p.total) },
                  ]}
                />
              )
            }}
          />
          <Area type="monotone" dataKey="contributions" stackId="1" stroke="var(--series-1)" strokeWidth={2} fill="var(--series-1)" fillOpacity={0.18} />
          <Area type="monotone" dataKey="growth" stackId="1" stroke="var(--series-3)" strokeWidth={2} fill="var(--series-3)" fillOpacity={0.18} />
        </AreaChart>
      </ResponsiveContainer>
    </div>
  )
}
