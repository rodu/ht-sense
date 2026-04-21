import { playwrightLauncher } from '@web/test-runner-playwright';
import { rollupAdapter } from '@web/dev-server-rollup';
import typescript from '@rollup/plugin-typescript';

export default {
  files: 'src/**/test/**/*.test.ts',
  nodeResolve: true,
  browsers: [playwrightLauncher({ product: 'chromium' })],
  plugins: [
    // Transform TypeScript using tsc (respects emitDecoratorMetadata and
    // experimentalDecorators — esbuild does not support these).
    rollupAdapter(typescript({ tsconfig: './tsconfig.test.json' })),
    // Note: @rollup/plugin-commonjs is intentionally omitted.
    // All Aurelia 1.x packages ship native ESM builds (the "module" field),
    // which WTR's nodeResolve picks up automatically.
  ],
};
