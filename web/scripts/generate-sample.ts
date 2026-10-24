// Writes the sample dataset as CSV files the C++ server can load.
// usage: npm run sample  (writes to ../data)

import { mkdirSync, writeFileSync } from 'node:fs'
import { join } from 'node:path'
import { generateSample } from '../src/lib/sampleData.ts'

const outDir = process.argv[2] ?? '../data'
const data = generateSample()

const money = (cents: number) => `${cents < 0 ? '-' : ''}$${(Math.abs(cents) / 100).toFixed(2)}`

mkdirSync(outDir, { recursive: true })
for (const account of data.accounts) {
  const rows = data.transactions
    .filter((t) => t.account === account.name)
    .map((t) => [t.id, t.date, account.name, t.category, money(t.amount), t.note].join(','))
  const csv = [`#AccountType=${account.type}`, 'ID,Date,Account,Category,Amount,Note', ...rows].join('\n') + '\n'
  writeFileSync(join(outDir, `${account.name}.csv`), csv)
  console.log(`${account.name}.csv  ${rows.length} transactions`)
}
writeFileSync(
  join(outDir, 'budgets.csv'),
  ['Category,Limit', ...data.budgets.map((b) => `${b.category},${money(b.limit)}`)].join('\n') + '\n',
)
console.log('budgets.csv')
