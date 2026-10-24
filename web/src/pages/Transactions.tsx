import { Download, Search } from 'lucide-react'
import { useMemo, useState } from 'react'
import { Button, Card, CategoryTag, Input, Select } from '../components/ui.tsx'
import { dateLabel, money, monthLabel, signedMoney } from '../lib/format.ts'
import { useFinance } from '../lib/store.tsx'
import { CATEGORIES } from '../lib/types.ts'

const PAGE = 40

export function Transactions() {
  const { data, months } = useFinance()
  const [query, setQuery] = useState('')
  const [account, setAccount] = useState('all')
  const [category, setCategory] = useState('all')
  const [month, setMonth] = useState('all')
  const [shown, setShown] = useState(PAGE)

  const rows = useMemo(() => {
    const q = query.trim().toLowerCase()
    return data.transactions
      .filter((t) => (account === 'all' || t.account === account) && (category === 'all' || t.category === category))
      .filter((t) => month === 'all' || t.date.startsWith(month))
      .filter((t) => !q || t.note.toLowerCase().includes(q) || t.id.toLowerCase().includes(q))
      .sort((a, b) => b.date.localeCompare(a.date) || b.id.localeCompare(a.id))
  }, [data.transactions, query, account, category, month])

  const inflow = rows.filter((t) => t.amount > 0).reduce((s, t) => s + t.amount, 0)
  const outflow = rows.filter((t) => t.amount < 0).reduce((s, t) => s - t.amount, 0)

  const exportCsv = () => {
    const esc = (s: string) => (/[",\n]/.test(s) ? `"${s.replace(/"/g, '""')}"` : s)
    const csv = ['ID,Date,Account,Category,Amount,Note', ...rows.map((t) => [t.id, t.date, t.account, t.category, (t.amount / 100).toFixed(2), t.note].map(esc).join(','))].join('\n')
    const url = URL.createObjectURL(new Blob([csv], { type: 'text/csv' }))
    const a = Object.assign(document.createElement('a'), { href: url, download: 'fintrack-transactions.csv' })
    a.click()
    URL.revokeObjectURL(url)
  }

  return (
    <Card>
      <div className="mb-4 grid grid-cols-1 gap-2 sm:grid-cols-2 lg:grid-cols-[1fr_auto_auto_auto_auto]">
        <div className="relative">
          <Search size={15} className="pointer-events-none absolute top-1/2 left-3 -translate-y-1/2 text-ink-3" />
          <Input placeholder="Search descriptions" value={query} onChange={(e) => { setQuery(e.target.value); setShown(PAGE) }} className="pl-9" />
        </div>
        <Select value={account} onChange={(e) => { setAccount(e.target.value); setShown(PAGE) }} aria-label="Account">
          <option value="all">All accounts</option>
          {data.accounts.map((a) => <option key={a.name}>{a.name}</option>)}
        </Select>
        <Select value={category} onChange={(e) => { setCategory(e.target.value); setShown(PAGE) }} aria-label="Category">
          <option value="all">All categories</option>
          {CATEGORIES.map((c) => <option key={c}>{c}</option>)}
        </Select>
        <Select value={month} onChange={(e) => { setMonth(e.target.value); setShown(PAGE) }} aria-label="Month">
          <option value="all">All months</option>
          {[...months].reverse().map((m) => <option key={m} value={m}>{monthLabel(m, 'long')}</option>)}
        </Select>
        <Button variant="secondary" onClick={exportCsv}><Download size={15} /> Export</Button>
      </div>

      <div className="mb-3 flex flex-wrap gap-x-6 gap-y-1 text-[13px] text-ink-3">
        <span><span className="num font-medium text-ink">{rows.length}</span> transactions</span>
        <span>In <span className="num font-medium text-positive">{money(inflow)}</span></span>
        <span>Out <span className="num font-medium text-ink">{money(outflow)}</span></span>
      </div>

      <div className="-mx-5 overflow-x-auto">
        <table className="w-full min-w-[640px] text-[13px]">
          <thead>
            <tr className="border-y border-line bg-surface-2 text-left text-[12px] font-medium tracking-wide text-ink-3 uppercase">
              <th className="px-5 py-2.5 font-medium">Date</th>
              <th className="px-3 py-2.5 font-medium">Description</th>
              <th className="px-3 py-2.5 font-medium">Category</th>
              <th className="px-3 py-2.5 font-medium">Account</th>
              <th className="px-5 py-2.5 text-right font-medium">Amount</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-line">
            {rows.slice(0, shown).map((t) => (
              <tr key={t.account + t.id} className="hover:bg-surface-2/60">
                <td className="num px-5 py-3 whitespace-nowrap text-ink-2">{dateLabel(t.date)}</td>
                <td className="px-3 py-3 font-medium">{t.note || '—'}</td>
                <td className="px-3 py-3"><CategoryTag category={t.category} /></td>
                <td className="px-3 py-3 whitespace-nowrap text-ink-2">{t.account}</td>
                <td className={`num px-5 py-3 text-right font-medium whitespace-nowrap ${t.amount > 0 ? 'text-positive' : ''}`}>{signedMoney(t.amount)}</td>
              </tr>
            ))}
          </tbody>
        </table>
        {rows.length === 0 && <p className="py-12 text-center text-ink-3">No transactions match these filters.</p>}
      </div>
      {shown < rows.length && (
        <div className="mt-4 flex justify-center">
          <Button variant="secondary" onClick={() => setShown((s) => s + PAGE)}>Show more ({rows.length - shown} left)</Button>
        </div>
      )}
    </Card>
  )
}
