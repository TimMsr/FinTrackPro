import { useState, type FormEvent } from 'react'
import { parseCents, todayIso } from '../lib/format.ts'
import { useFinance } from '../lib/store.tsx'
import { CATEGORIES, type AccountType, type Category } from '../lib/types.ts'
import { Button, Field, Input, Modal, Segmented, Select } from './ui.tsx'

export function TransactionForm({ onClose }: { onClose: () => void }) {
  const { data, run } = useFinance()
  const [kind, setKind] = useState<'expense' | 'income'>('expense')
  const [account, setAccount] = useState(data.accounts[0]?.name ?? '')
  const [amount, setAmount] = useState('')
  const [date, setDate] = useState(todayIso())
  const [category, setCategory] = useState<Category>('Food')
  const [note, setNote] = useState('')
  const [error, setError] = useState('')

  const submit = async (e: FormEvent) => {
    e.preventDefault()
    const cents = parseCents(amount)
    if (!Number.isFinite(cents) || cents <= 0) return setError('Enter an amount greater than zero')
    const ok = await run(
      (api) => api.addTransaction({ account, date, amount: kind === 'expense' ? -cents : cents, category, note: note.trim() }),
      'Transaction added',
    )
    if (ok) onClose()
  }

  return (
    <Modal title="Add transaction" onClose={onClose}>
      <form onSubmit={submit} className="flex flex-col gap-4">
        <Segmented
          value={kind}
          onChange={(k) => {
            setKind(k)
            setCategory(k === 'income' ? 'Income' : 'Food')
          }}
          options={[{ value: 'expense', label: 'Expense' }, { value: 'income', label: 'Income' }]}
        />
        <div className="grid grid-cols-2 gap-3">
          <Field label="Amount">
            <Input inputMode="decimal" placeholder="0.00" value={amount} onChange={(e) => { setAmount(e.target.value); setError('') }} autoFocus />
          </Field>
          <Field label="Date">
            <Input type="date" value={date} onChange={(e) => setDate(e.target.value)} required />
          </Field>
        </div>
        <div className="grid grid-cols-2 gap-3">
          <Field label="Account">
            <Select value={account} onChange={(e) => setAccount(e.target.value)}>
              {data.accounts.map((a) => <option key={a.name}>{a.name}</option>)}
            </Select>
          </Field>
          <Field label="Category">
            <Select value={category} onChange={(e) => setCategory(e.target.value as Category)}>
              {CATEGORIES.filter((c) => c !== 'Transfer').map((c) => <option key={c}>{c}</option>)}
            </Select>
          </Field>
        </div>
        <Field label="Description">
          <Input placeholder="e.g. Groceries · Green Market" value={note} onChange={(e) => setNote(e.target.value)} maxLength={80} />
        </Field>
        {error && <p className="text-[13px] text-negative">{error}</p>}
        <div className="mt-1 flex justify-end gap-2">
          <Button type="button" variant="secondary" onClick={onClose}>Cancel</Button>
          <Button type="submit">Add transaction</Button>
        </div>
      </form>
    </Modal>
  )
}

export function TransferForm({ onClose }: { onClose: () => void }) {
  const { data, run } = useFinance()
  const names = data.accounts.map((a) => a.name)
  const [from, setFrom] = useState(names[0] ?? '')
  const [to, setTo] = useState(names[1] ?? '')
  const [amount, setAmount] = useState('')
  const [date, setDate] = useState(todayIso())
  const [error, setError] = useState('')

  const submit = async (e: FormEvent) => {
    e.preventDefault()
    const cents = parseCents(amount)
    if (!Number.isFinite(cents) || cents <= 0) return setError('Enter an amount greater than zero')
    if (from === to) return setError('Choose two different accounts')
    if (await run((api) => api.transfer({ from, to, date, amount: cents }), 'Transfer completed')) onClose()
  }

  return (
    <Modal title="Transfer between accounts" onClose={onClose}>
      <form onSubmit={submit} className="flex flex-col gap-4">
        <div className="grid grid-cols-2 gap-3">
          <Field label="From">
            <Select value={from} onChange={(e) => { setFrom(e.target.value); setError('') }}>
              {names.map((n) => <option key={n}>{n}</option>)}
            </Select>
          </Field>
          <Field label="To">
            <Select value={to} onChange={(e) => { setTo(e.target.value); setError('') }}>
              {names.map((n) => <option key={n}>{n}</option>)}
            </Select>
          </Field>
        </div>
        <div className="grid grid-cols-2 gap-3">
          <Field label="Amount">
            <Input inputMode="decimal" placeholder="0.00" value={amount} onChange={(e) => { setAmount(e.target.value); setError('') }} autoFocus />
          </Field>
          <Field label="Date">
            <Input type="date" value={date} onChange={(e) => setDate(e.target.value)} required />
          </Field>
        </div>
        {error && <p className="text-[13px] text-negative">{error}</p>}
        <div className="mt-1 flex justify-end gap-2">
          <Button type="button" variant="secondary" onClick={onClose}>Cancel</Button>
          <Button type="submit">Transfer</Button>
        </div>
      </form>
    </Modal>
  )
}

export function AccountForm({ onClose }: { onClose: () => void }) {
  const { run } = useFinance()
  const [name, setName] = useState('')
  const [type, setType] = useState<AccountType>('checking')

  const submit = async (e: FormEvent) => {
    e.preventDefault()
    if (await run((api) => api.addAccount(name.trim(), type), 'Account created')) onClose()
  }

  return (
    <Modal title="New account" onClose={onClose}>
      <form onSubmit={submit} className="flex flex-col gap-4">
        <Field label="Account type">
          <Segmented
            value={type}
            onChange={setType}
            options={[{ value: 'checking', label: 'Checking' }, { value: 'savings', label: 'Savings' }, { value: 'credit', label: 'Credit' }]}
          />
        </Field>
        <Field label="Name">
          <Input placeholder="e.g. Travel Fund" value={name} onChange={(e) => setName(e.target.value)} pattern="[A-Za-z0-9 _\-]{1,40}" title="Letters, numbers, spaces, - and _" required autoFocus />
        </Field>
        <div className="mt-1 flex justify-end gap-2">
          <Button type="button" variant="secondary" onClick={onClose}>Cancel</Button>
          <Button type="submit">Create account</Button>
        </div>
      </form>
    </Modal>
  )
}
