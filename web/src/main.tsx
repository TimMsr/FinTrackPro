import '@fontsource-variable/inter'
import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import App from './App.tsx'
import './index.css'
import { FinanceProvider } from './lib/store.tsx'

const Loading = () => (
  <div className="grid min-h-screen place-items-center text-[14px] text-ink-3">Loading your finances…</div>
)

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <FinanceProvider fallback={<Loading />}>
      <App />
    </FinanceProvider>
  </StrictMode>,
)
