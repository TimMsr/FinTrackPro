import { ArrowDownRight, ArrowUpRight, LifeBuoy, PiggyBank, Receipt, TrendingUp } from 'lucide-react'
import { useMemo, useState } from 'react'
import { ProjectionChart } from '../components/charts.tsx'
import { Card, CATEGORY_COLOR, StatCard } from '../components/ui.tsx'
import { average, categoryTrend, emergencyRunway, monthlyFlows, netWorthSeries, projectSavings } from '../lib/analytics.ts'
import { money, moneyWhole, monthLabel, percent, todayIso } from '../lib/format.ts'
import { useFinance } from '../lib/store.tsx'

function Slider({ label, value, display, min, max, step, onChange }: {
  label: string
  value: number
  display: string
  min: number
  max: number
  step: number
  onChange: (v: number) => void
}) {
  return (
    <label className="flex flex-col gap-2">
      <span className="flex items-baseline justify-between text-[13px]">
        <span className="font-medium text-ink-2">{label}</span>
        <span className="num font-semibold">{display}</span>
      </span>
      <input type="range" min={min} max={max} step={step} value={value} onChange={(e) => onChange(Number(e.target.value))} />
    </label>
  )
}

export function Insights() {
  const { data, month } = useFinance()
  const flows = useMemo(() => monthlyFlows(data.transactions), [data.transactions])
  const worth = useMemo(() => netWorthSeries(data.transactions, data.accounts), [data])
  const trend = useMemo(() => categoryTrend(data.transactions, month), [data.transactions, month])
  const runway = emergencyRunway(data.accounts, flows)

  const avgRate = average(flows.map((f) => f.savingsRate))
  const avgSpend = average(flows.map((f) => f.expenses))
  const avgNet = Math.max(0, Math.round(average(flows.map((f) => f.net)) / 10000) * 100)
  const growth = worth.length > 1 ? worth.at(-1)!.netWorth - worth[0].netWorth : 0
  const savings = data.accounts.filter((a) => a.type === 'savings')
  const savingsBalance = savings.reduce((s, a) => s + a.balance, 0)

  const [monthly, setMonthly] = useState(avgNet)
  const [rate, setRate] = useState(0.07)
  const [years, setYears] = useState(10)
  const projection = projectSavings(savingsBalance, monthly * 100, rate, years)
  const final = projection.at(-1)!

  return (
    <div className="flex flex-col gap-5">
      <div className="grid grid-cols-1 gap-4 sm:grid-cols-2 xl:grid-cols-4">
        <StatCard label="Avg. savings rate" icon={<PiggyBank size={16} />} value={percent(avgRate)} footer={<span className="text-[13px] text-ink-3">Over {flows.length} months</span>} />
        <StatCard label="Emergency runway" icon={<LifeBuoy size={16} />} value={`${runway.months.toFixed(1)} mo`} footer={<span className="text-[13px] text-ink-3">Cash + savings ÷ monthly spend</span>} />
        <StatCard label="Avg. monthly spend" icon={<Receipt size={16} />} value={moneyWhole(avgSpend)} footer={<span className="text-[13px] text-ink-3">Excluding transfers</span>} />
        <StatCard label="Net worth growth" icon={<TrendingUp size={16} />} value={moneyWhole(growth)} footer={<span className="text-[13px] text-ink-3">Since {monthLabel(worth[0]?.month ?? month, 'long')}</span>} />
      </div>

      <div className="grid grid-cols-1 gap-5 lg:grid-cols-3">
        <Card title="Savings projection" subtitle="Monthly compounding on your current savings balance" className="lg:col-span-2">
          <div className="mb-5 flex flex-wrap items-end gap-x-8 gap-y-2">
            <div>
              <div className="text-[13px] text-ink-3">Projected value in {years} years</div>
              <div className="num text-[28px] font-semibold tracking-tight">{moneyWhole(final.total)}</div>
            </div>
            <div className="text-[13px] text-ink-3">
              <span className="num font-medium text-ink">{moneyWhole(final.growth)}</span> from compound growth ·{' '}
              <span className="num font-medium text-ink">{percent(final.total ? final.growth / final.total : 0, 0)}</span> of total
            </div>
          </div>
          <ProjectionChart data={projection} />
        </Card>
        <Card title="Assumptions" subtitle={`Starting from ${money(savingsBalance)} in savings`}>
          <div className="flex flex-col gap-6">
            <Slider label="Monthly contribution" value={monthly} display={moneyWhole(monthly * 100)} min={0} max={5000} step={50} onChange={setMonthly} />
            <Slider label="Annual return" value={rate} display={percent(rate)} min={0} max={0.12} step={0.005} onChange={setRate} />
            <Slider label="Time horizon" value={years} display={`${years} years`} min={1} max={40} step={1} onChange={setYears} />
            <p className="rounded-lg bg-surface-2 p-3 text-[12px] leading-relaxed text-ink-3">
              Default contribution is your average monthly net cash flow ({moneyWhole(avgNet * 100)}). 7% approximates the long-run real return of a
              diversified equity index; a high-yield savings account earns closer to {percent(savings[0]?.interestRate ?? 0.05, 0)}.
            </p>
          </div>
        </Card>
      </div>

      <Card title="Spending vs. your recent average" subtitle={`${monthLabel(month, 'long')}${todayIso().startsWith(month) ? ' (month to date)' : ''} compared with the three months before`}>
        <div className="-mx-5 overflow-x-auto">
          <table className="w-full min-w-[520px] text-[13px]">
            <thead>
              <tr className="border-y border-line bg-surface-2 text-left text-[12px] tracking-wide text-ink-3 uppercase">
                <th className="px-5 py-2.5 font-medium">Category</th>
                <th className="px-3 py-2.5 text-right font-medium">This month</th>
                <th className="px-3 py-2.5 text-right font-medium">3-mo average</th>
                <th className="px-5 py-2.5 text-right font-medium">Change</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-line">
              {trend.map((t) => {
                const up = t.change > 0.005
                const down = t.change < -0.005
                return (
                  <tr key={t.category}>
                    <td className="px-5 py-3">
                      <span className="flex items-center gap-2 font-medium">
                        <span className="size-2 rounded-full" style={{ background: CATEGORY_COLOR[t.category] }} />
                        {t.category}
                      </span>
                    </td>
                    <td className="num px-3 py-3 text-right">{money(t.current)}</td>
                    <td className="num px-3 py-3 text-right text-ink-2">{money(t.average)}</td>
                    <td className="px-5 py-3 text-right">
                      <span className={`num inline-flex items-center gap-0.5 font-medium ${up ? 'text-negative' : down ? 'text-positive' : 'text-ink-3'}`}>
                        {up ? <ArrowUpRight size={14} /> : down ? <ArrowDownRight size={14} /> : null}
                        {up || down ? percent(Math.abs(t.change)) : 'Flat'}
                      </span>
                    </td>
                  </tr>
                )
              })}
            </tbody>
          </table>
        </div>
      </Card>
    </div>
  )
}
