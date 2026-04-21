import { defineConfig } from 'vite';
import { nodePolyfills } from 'vite-plugin-node-polyfills';

export default defineConfig({
  server: {
    open: !process.env.CI,
    port: 9000,
  },
  esbuild: {
    target: 'es2022',
  },
  plugins: [nodePolyfills()],
});
