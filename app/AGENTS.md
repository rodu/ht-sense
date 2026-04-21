# HT Sense Project

The HT Sense is a data visualization dashboard to show humidity and temperature
data collected from an Arduino project.

This is a hobby project for learning Arduino and data visualization.

## Project Organization

The solution is a client side web application using the Lit library to render components.

Mostly the application presents a simple dashboard based
on the EChart.js library (https://echarts.apache.org/) to create a gauge component
used to render the humidity and temperature data collected by a sensor connected
to the Arduino board.

Data will come in as a continuous stream to be consumed using RxJS and rendered
in a Lit element with the gauge component.

The client communicates with the server by subscribing to an MQTT topic using
the MQTT.js library (https://github.com/mqttjs/MQTT.js#readme) to handle the low
level communication.

### MQTT Broker

The MQTT broker is based on a Docker container running EMQX (https://github.com/emqx/emqx)

```shell
docker run -d --name emqx \
  -p 1883:1883 -p 8083:8083 -p 8084:8084 \
  -p 8883:8883 -p 18083:18083 \
  emqx/emqx-enterprise:latest
```

The MQTT broker receives a stream of data from the Arduino R4 WiFi board and
accepts subscriptions (from clients) that intend to consume the data stream.

Events from the Arduino board and later toward the clients can be dispatched
with QoS 0 (fire and forget).

## Commands

```bash
npm run dev      # Dev server on port 9000 (auto-opens browser)
npm run build    # tsc && vite build
npm run preview  # Preview production build
```

## Architecture

```
src/
  main.ts              # Bootstrap: import order is critical (see below)
  app.ts               # Root component <my-element>
  index.css            # Global styles
  core/
    di.ts              # Creates and globalizes the Aurelia DI container
    decorators.ts      # Custom @instance property-injection decorator
  components/          # Lit Web Components
  services/            # Singleton services resolved via DI
```

## Key Patterns

### Initialization Order (main.ts)

Always import in this order — metadata must be available before DI and decorators run:

```ts
import 'reflect-metadata'; // 1. Decorator metadata polyfill
import './core/di'; // 2. Create & globalize DI container
import './core/decorators'; // 3. Register custom decorators
import './app'; // 4. Root component
```

### Dependency Injection — `@instance`

Services are injected as class properties using the custom `@instance` decorator, **not** via constructors:

```ts
import { instance } from '../core/decorators';
import { ApiService } from '../services/api-service';

class MyComponent extends LitElement {
  @instance({ type: ApiService }) private apiService!: ApiService;
}
```

The Aurelia `Container` is a singleton (set global in `di.ts`). Services registered with it are singletons by default.

### Creating a New Service

Place services in `src/services/`. Services are plain classes — the DI container resolves them automatically:

```ts
// src/services/my-service.ts
export class MyService {
  doSomething() { ... }
}
```

Inject with `@instance({ type: MyService })` in any component.

### Creating a New Component

Place Lit components in `src/components/`. Use `@customElement` and Lit's `@property` / `@state` decorators:

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

### Cross-Component Communication — EventAggregator

Use Aurelia's `EventAggregator` (not native DOM events) for cross-component messaging:

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
    this.subscription?.dispose(); // Always dispose subscriptions
  }
}
```

## TypeScript Notes

- `experimentalDecorators: true` and `emitDecoratorMetadata: true` are required — do not remove them from `tsconfig.json`.
- `useDefineForClassFields: false` is required for Aurelia metadata reflection to work with class properties.
- `strict` mode is enabled.

## Vite Config

The config file is named `xvite.config.ts` (not `vite.config.ts`). Uses `vite-plugin-node-polyfills` for Node.js compatibility.

## Testing

Tests use `@web/test-runner` (WTR) running against real Chromium via Playwright, with TypeScript compiled through `@rollup/plugin-typescript` (not esbuild — esbuild does not support `emitDecoratorMetadata`). The test setup is separate from the Vite build.

```bash
npm run test        # Run all tests once
npm run test:watch  # Re-run on file changes
```

Test files are co-located with components and named `*.test.ts`. The WTR config is `web-test-runner.config.js`.

### Required Import Order in Test Files

`src/test/setup.ts` **must be the very first import** in every test file. It bootstraps `reflect-metadata` and the DI container, and pre-registers all injectable services before any component module is evaluated:

```ts
import '../test/setup'; // MUST be first
import { fixture, html, expect, elementUpdated } from '@open-wc/testing';
import './my-component';
```

### Why setup.ts Must Come First

The `@instance` decorator calls `Container.get()` at class-definition time (when a component module is first evaluated by the browser). If a service is not already registered, Aurelia falls back to `autoRegister()`, which calls `Reflect.getOwnMetadata()` from `reflect-metadata`. Because the browser's ES module loader does not guarantee evaluation order for sibling imports, `reflect-metadata` may not have run yet — causing a runtime error.

`setup.ts` solves this by calling `Container.instance.registerInstance(ServiceClass, new ServiceClass())` for every injectable service before any component module is imported. `registerInstance` stores a direct-return resolver that bypasses `autoRegister()` entirely.

### Adding a New Service to Tests

When a new service is added to the project, register it in `src/test/setup.ts`:

```ts
import { MyService } from '../services/my-service';

Container.instance.registerInstance(MyService, new MyService());
```

### Writing Tests

Tests use Mocha (`describe`/`it`) and Chai assertions. Use `@open-wc/testing` helpers to render components and interact with them:

```ts
import '../test/setup';
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

To test EventAggregator interactions, retrieve the shared instance from the container:

```ts
import { Container } from 'aurelia-dependency-injection';
import { EventAggregator } from 'aurelia-event-aggregator';

const ea = Container.instance.get(EventAggregator);
const sub = ea.subscribe('MY_EVENT', (data) => {
  /* ... */
});
// ... trigger the event, then:
sub.dispose();
```
