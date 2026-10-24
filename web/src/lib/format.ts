const usd = new Intl.NumberFormat('en-US', { style: 'currency', currency: 'USD' })
const usdWhole = new Intl.NumberFormat('en-US', { style: 'currency', currency: 'USD', maximumFractionDigits: 0 })
const usdCompact = new Intl.NumberFormat('en-US', { style: 'currency', currency: 'USD', notation: 'compact', maximumFractionDigits: 1 })

export const money = (cents: number) => usd.format(cents / 100)
export const moneyWhole = (cents: number) => usdWhole.format(cents / 100)
export const moneyCompact = (cents: number) => usdCompact.format(cents / 100)
export const signedMoney = (cents: number) => (cents > 0 ? '+' : '') + money(cents)
export const percent = (value: number, digits = 1) => `${(value * 100).toFixed(digits)}%`

export function monthLabel(key: string, style: 'short' | 'long' = 'short') {
  const [y, m] = key.split('-').map(Number)
  return new Date(y, m - 1, 1).toLocaleDateString('en-US', style === 'short' ? { month: 'short' } : { month: 'long', year: 'numeric' })
}

export function dateLabel(date: string) {
  const [y, m, d] = date.split('-').map(Number)
  return new Date(y, m - 1, d).toLocaleDateString('en-US', { month: 'short', day: 'numeric', year: 'numeric' })
}

export const todayIso = () => {
  const d = new Date()
  return `${d.getFullYear()}-${String(d.getMonth() + 1).padStart(2, '0')}-${String(d.getDate()).padStart(2, '0')}`
}

// parse "12.34" / "$1,234.5" into integer cents, NaN when invalid
export function parseCents(text: string) {
  const cleaned = text.replace(/[$,\s]/g, '')
  if (!/^\d+(\.\d{0,2})?$/.test(cleaned)) return NaN
  return Math.round(Number(cleaned) * 100)
}
