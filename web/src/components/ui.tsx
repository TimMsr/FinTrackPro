import { X } from 'lucide-react'
import { useEffect, type ReactNode } from 'react'
import type { Category } from '../lib/types.ts'

export const CATEGORY_COLOR: Record<Category, string> = {
  Rent: 'var(--series-1)',
  Food: 'var(--series-2)',
  Utilities: 'var(--series-3)',
  Transport: 'var(--series-4)',
  Other: 'var(--series-5)',
  Income: 'var(--neutral)',
  Transfer: 'var(--neutral)',
}

export function Card({ title, subtitle, action, children, className = '' }: {
  title?: string
  subtitle?: string
  action?: ReactNode
  children: ReactNode
  className?: string
}) {
  return (
    <section className={`card p-5 ${className}`}>
      {(title || action) && (
        <header className="mb-4 flex items-start justify-between gap-3">
          <div>
            {title && <h2 className="text-[15px] font-semibold tracking-tight text-ink">{title}</h2>}
            {subtitle && <p className="mt-0.5 text-[13px] text-ink-3">{subtitle}</p>}
          </div>
          {action}
        </header>
      )}
      {children}
    </section>
  )
}

export function Delta({ value, invert = false, suffix = 'vs last month' }: { value: number; invert?: boolean; suffix?: string }) {
  if (!Number.isFinite(value) || value === 0) return <span className="text-[13px] text-ink-3">No change {suffix}</span>
  const good = invert ? value < 0 : value > 0
  return (
    <span className="text-[13px] text-ink-3">
      <span className={`num font-medium ${good ? 'text-positive' : 'text-negative'}`}>
        {value > 0 ? '▲' : '▼'} {Math.abs(value * 100).toFixed(1)}%
      </span>{' '}
      {suffix}
    </span>
  )
}

export function StatCard({ label, value, footer, icon }: { label: string; value: string; footer?: ReactNode; icon?: ReactNode }) {
  return (
    <div className="card flex flex-col gap-2 p-5">
      <div className="flex items-center justify-between text-[13px] font-medium text-ink-2">
        {label}
        <span className="text-ink-3">{icon}</span>
      </div>
      <div className="num text-[28px] font-semibold leading-none tracking-tight">{value}</div>
      {footer && <div>{footer}</div>}
    </div>
  )
}

export function Button({ variant = 'primary', className = '', ...props }: React.ButtonHTMLAttributes<HTMLButtonElement> & { variant?: 'primary' | 'secondary' | 'ghost' }) {
  const styles = {
    primary: 'bg-accent text-accent-ink hover:brightness-110',
    secondary: 'border border-line bg-surface text-ink hover:bg-surface-2',
    ghost: 'text-ink-2 hover:bg-surface-2 hover:text-ink',
  }[variant]
  return (
    <button
      className={`inline-flex h-9 items-center justify-center gap-1.5 rounded-lg px-3.5 text-[13px] font-medium transition disabled:opacity-50 focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-accent ${styles} ${className}`}
      {...props}
    />
  )
}

const fieldClass =
  'h-9 w-full rounded-lg border border-line bg-surface px-3 text-[13px] text-ink placeholder:text-ink-3 focus:border-accent focus:outline-none focus:ring-3 focus:ring-accent/20'

export function Input(props: React.InputHTMLAttributes<HTMLInputElement>) {
  return <input {...props} className={`${fieldClass} ${props.className ?? ''}`} />
}

export function Select({ className = '', ...props }: React.SelectHTMLAttributes<HTMLSelectElement>) {
  return <select {...props} className={`${fieldClass} pr-8 ${className}`} />
}

export function Field({ label, children }: { label: string; children: ReactNode }) {
  return (
    <label className="flex flex-col gap-1.5 text-[13px] font-medium text-ink-2">
      {label}
      {children}
    </label>
  )
}

export function CategoryTag({ category }: { category: Category }) {
  return (
    <span className="inline-flex items-center gap-1.5 rounded-full border border-line px-2 py-0.5 text-[12px] font-medium text-ink-2">
      <span className="size-2 rounded-full" style={{ background: CATEGORY_COLOR[category] }} />
      {category}
    </span>
  )
}

export function Modal({ title, onClose, children }: { title: string; onClose: () => void; children: ReactNode }) {
  useEffect(() => {
    const onKey = (e: KeyboardEvent) => e.key === 'Escape' && onClose()
    window.addEventListener('keydown', onKey)
    return () => window.removeEventListener('keydown', onKey)
  }, [onClose])
  return (
    <div className="fixed inset-0 z-50 flex items-end justify-center bg-black/40 p-4 backdrop-blur-[2px] sm:items-center" onMouseDown={onClose}>
      <div role="dialog" aria-modal="true" aria-label={title} className="card w-full max-w-md p-6" onMouseDown={(e) => e.stopPropagation()}>
        <div className="mb-5 flex items-center justify-between">
          <h2 className="text-base font-semibold">{title}</h2>
          <button onClick={onClose} className="rounded-md p-1 text-ink-3 hover:bg-surface-2 hover:text-ink" aria-label="Close">
            <X size={18} />
          </button>
        </div>
        {children}
      </div>
    </div>
  )
}

export function Segmented<T extends string>({ value, options, onChange }: { value: T; options: { value: T; label: string }[]; onChange: (v: T) => void }) {
  return (
    <div className="inline-flex rounded-lg border border-line bg-surface-2 p-0.5">
      {options.map((o) => (
        <button
          key={o.value}
          type="button"
          onClick={() => onChange(o.value)}
          className={`h-8 flex-1 rounded-md px-3 text-[13px] font-medium transition ${value === o.value ? 'bg-surface text-ink shadow-sm' : 'text-ink-3 hover:text-ink'}`}
        >
          {o.label}
        </button>
      ))}
    </div>
  )
}
