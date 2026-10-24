import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import tailwindcss from '@tailwindcss/vite'

// `npm run build:demo` produces the static GitHub Pages build (no backend, sample data in the browser).
// BASE_PATH lets the Pages workflow serve it from /<repo>/.
export default defineConfig(({ mode }) => ({
  base: mode === 'demo' ? process.env.BASE_PATH ?? './' : '/',
  plugins: [react(), tailwindcss()],
  build: { chunkSizeWarningLimit: 800 },
  server: {
    proxy: { '/api': 'http://127.0.0.1:8080' },
  },
}))
