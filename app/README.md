# Vite + Lit Starter Template

A starter template for building modern UI applications with [Lit](https://lit.dev/) Web Components, [Vite](https://vitejs.dev/), and [Aurelia's](https://aurelia.io/) Dependency Injection and EventAggregator.

The included demo renders a list of users fetched from a mock API service, with click-to-select functionality wired through the EventAggregator.

Two Aurelia primitives power the architecture:

- **`Container`** — an Inversion of Control (IoC) container that manages service lifetimes and automatically injects dependencies into components, keeping them decoupled from construction concerns.
- **`EventAggregator`** — a pub/sub message bus that allows components to communicate without holding direct references to each other.

## Tech Stack

| Tool                                                                            | Purpose                                |
| ------------------------------------------------------------------------------- | -------------------------------------- |
| [Lit](https://lit.dev/)                                                         | Web Components with reactive rendering |
| [Vite](https://vitejs.dev/)                                                     | Fast dev server and bundler            |
| [aurelia-dependency-injection](https://github.com/aurelia/dependency-injection) | Property-based DI container            |
| [aurelia-event-aggregator](https://github.com/aurelia/event-aggregator)         | Pub/sub cross-component messaging      |

## Getting Started

```bash
npm install
npm run dev      # Start dev server at http://localhost:9000
```

## Commands

| Command              | Description                                        |
| -------------------- | -------------------------------------------------- |
| `npm run dev`        | Start dev server (auto-opens browser on port 9000) |
| `npm run build`      | Type-check and build for production                |
| `npm run preview`    | Preview the production build locally               |
| `npm run test`       | Run all tests once (headless Chromium)             |
| `npm run test:watch` | Re-run tests on file changes                       |

## Project Structure

```
src/
  main.ts              # Entry point — import order is critical
  app.ts               # Root component <my-element>
  index.css            # Global styles
  core/
    di.ts              # Creates and globalizes the Aurelia DI container
    decorators.ts      # Custom @instance property-injection decorator
  components/          # Lit Web Components
  services/            # Singleton services resolved via DI
```

## Key Patterns

### Dependency Injection — Aurelia `Container` + `@instance`

This project uses Aurelia's `Container` as an IoC container to manage service instances and their dependencies. Rather than manually constructing services or passing them through component trees, the container resolves and provides them automatically. This brings several concrete benefits:

- **Decoupling** — components declare what they need, not how to build it.
- **Singleton management** — the container ensures a single shared instance of each service across the entire application.
- **Testability** — services can be swapped out or mocked at the container level without touching component code.
- **Scalability** — adding a new dependency to a component is a one-line change regardless of how complex the service's own dependencies are.

Because Lit Web Components are not instantiated by the container (the browser creates them), a custom `@instance` property decorator is provided in `src/core/decorators.ts`. It hooks into the globally registered container at property initialisation time, making injection feel natural and requiring zero boilerplate beyond the decorator itself:

```ts
import { instance } from '../core/decorators';
import { ApiService } from '../services/api-service';

class MyComponent extends LitElement {
  @instance({ type: ApiService }) private apiService!: ApiService;
}
```

Services are singletons by default — the same instance is shared across all components.

### Adding a New Service

Create a plain class in `src/services/` and inject it anywhere with `@instance`:

```ts
// src/services/my-service.ts
export class MyService {
  doSomething() { ... }
}
```

### Adding a New Component

Create a Lit component in `src/components/`:

```ts
import { LitElement, html } from 'lit';
import { customElement, property } from 'lit/decorators.js';

@customElement('my-widget')
export class MyWidget extends LitElement {
  @property() label = '';
  render() {
    return html`<div>${this.label}</div>`;
  }
}
```

### Cross-Component Communication — Aurelia `EventAggregator`

Aurelia's `EventAggregator` is used as the primary mechanism for communication between components. It implements the pub/sub (publish/subscribe) pattern, acting as a central message bus: publishers fire named events with a payload, and any number of subscribers react to them — without either side holding a reference to the other.

This approach is preferred over native DOM events for complex inter-component communication because:

- **No shared references** — components never import or depend on each other directly.
- **Many-to-many** — multiple components can publish or subscribe to the same event independently.
- **Decoupled orchestration** — component interactions can be added, changed, or removed without modifying the components themselves.

The `EventAggregator` is itself resolved via the DI container using `@instance`:

```ts
import { EventAggregator } from 'aurelia-event-aggregator';
import { instance } from '../core/decorators';

class MyComponent extends LitElement {
  @instance({ type: EventAggregator }) private ea!: EventAggregator;

  connectedCallback() {
    super.connectedCallback();
    this.subscription = this.ea.subscribe('MY_EVENT', (data) => { ... });
  }

  disconnectedCallback() {
    super.disconnectedCallback();
    this.subscription?.dispose(); // Always dispose to avoid memory leaks
  }
}
```

## TypeScript Requirements

The following `tsconfig.json` settings are required and must not be removed:

```json
{
  "experimentalDecorators": true,
  "emitDecoratorMetadata": true,
  "useDefineForClassFields": false
}
```

`useDefineForClassFields: false` is necessary for Aurelia's metadata reflection to work correctly with class properties. `strict` mode is enabled.

## Testing

Tests run in a real Chromium browser (via Playwright) using [`@web/test-runner`](https://modern-web.dev/docs/test-runner/overview/). TypeScript is compiled by `@rollup/plugin-typescript` rather than esbuild, because esbuild does not support `emitDecoratorMetadata` — which Aurelia's DI requires.

Test files live in a `test/` subfolder under component paths (for example, `src/components/test/user-detail.test.ts`) and are named `*.test.ts`.

### Running tests

```bash
npm run test        # Run all tests once
npm run test:watch  # Re-run on file changes
```

### Writing a test

Use [Mocha](https://mochajs.org/) (`describe`/`it`) and [Chai](https://www.chaijs.com/) assertions. [`@open-wc/testing`](https://open-wc.org/docs/testing/testing-package/) provides the `fixture` and `elementUpdated` helpers for rendering and updating Lit components:

```ts
import '../test/setup'; // MUST be the first import — see below
import { fixture, html, expect, elementUpdated } from '@open-wc/testing';
import './my-component';
import type { MyComponent } from './my-component';

describe('MyComponent', () => {
  it('renders correctly', async () => {
    const el = await fixture<MyComponent>(html`<my-component></my-component>`);
    expect(el.shadowRoot!.querySelector('.my-class')).to.not.be.null;
  });

  it('updates on property change', async () => {
    const el = await fixture<MyComponent>(html`<my-component></my-component>`);
    el.label = 'hello';
    await elementUpdated(el);
    expect(el.shadowRoot!.textContent).to.include('hello');
  });
});
```

### The `src/test/setup.ts` bootstrap — why it must come first

`src/test/setup.ts` **must be the first import** in every test file. It handles two things:

1. Imports `reflect-metadata` to polyfill `Reflect.getOwnMetadata`.
2. Creates the Aurelia `Container` and pre-registers all injectable services.

**Why pre-registration matters:** the `@instance` decorator calls `Container.get()` at class-definition time, the moment a component module is first evaluated by the browser. If the service isn't already registered, Aurelia falls back to `autoRegister()`, which calls `Reflect.getOwnMetadata()`. Because the browser's ES module loader does not guarantee evaluation order for sibling imports, `reflect-metadata` may not have run yet — causing a runtime error.

The fix is `Container.instance.registerInstance(ServiceClass, new ServiceClass())` for every injectable service. This stores a direct-return resolver, so `autoRegister()` is never called.

### Adding a new service to tests

Whenever a new service is added to `src/services/`, register it in `src/test/setup.ts`:

```ts
import { MyService } from '../services/my-service';

Container.instance.registerInstance(MyService, new MyService());
```

### Testing EventAggregator interactions

Retrieve the shared `EventAggregator` instance from the container and subscribe before triggering the action under test:

```ts
import { Container } from 'aurelia-dependency-injection';
import { EventAggregator } from 'aurelia-event-aggregator';

const ea = Container.instance.get(EventAggregator);
const sub = ea.subscribe('MY_EVENT', (data) => {
  /* assert on data */
});
// ... trigger the event ...
sub.dispose();
```

## Notes

- The Vite config file is named `xvite.config.ts` (not `vite.config.ts`).
- Bootstrap import order in `main.ts` matters — `reflect-metadata` must be first, followed by `di`, `decorators`, then component imports.
