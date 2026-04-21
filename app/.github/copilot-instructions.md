# HT Sense Project

The HT Sense is a data visualization dashboard to show humidity and temperature
data collected from an Arduino project.

This is a hobby project for learning Arduino and data visualization.

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

## Project Structure

```
src/
  main.ts              # Bootstrap entry point — import order is critical
  app.ts               # Root component <my-element>
  index.css            # Global styles
  core/
    di.ts              # Creates and globalizes the Aurelia DI container
    decorators.ts      # Custom @instance property-injection decorator
  components/          # Lit Web Components
  services/            # Singleton services resolved via DI
```

The Vite config file is `xvite.config.ts` (not `vite.config.ts`).

## Bootstrap Order (main.ts)

Always import in this exact order — metadata must be available before DI and decorators run:

```ts
import 'reflect-metadata'; // 1. Decorator metadata polyfill
import './core/di'; // 2. Create & globalize DI container
import './core/decorators'; // 3. Register custom decorators
import './app'; // 4. Root component
```

## Dependency Injection

Inject services as class properties using the `@instance` decorator — never via constructors:

```ts
import { instance } from '../core/decorators';
import { MyService } from '../services/my-service';

class MyComponent extends LitElement {
  @instance({ type: MyService }) private myService!: MyService;
}
```

The Aurelia `Container` is a singleton. All services are singletons by default.

## Creating a New Service

Place services in `src/services/`. Services are plain classes — no registration needed:

```ts
// src/services/my-service.ts
export class MyService {
  doSomething() { ... }
}
```

## Creating a New Component

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

## Cross-Component Communication

Use Aurelia's `EventAggregator` for cross-component messaging — not native DOM events:

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

## TypeScript Requirements

- `experimentalDecorators: true` and `emitDecoratorMetadata: true` must remain in `tsconfig.json`
- `useDefineForClassFields: false` is required for Aurelia metadata reflection to work
- `strict` mode is enabled

## Testing

Tests use `@web/test-runner` (WTR) running in real Chromium via Playwright. TypeScript for tests is compiled with `@rollup/plugin-typescript` rather than esbuild, because esbuild does not support `emitDecoratorMetadata`. The test setup is separate from the Vite build.

Test files live in `test/` subfolders under component paths (for example, `src/components/test/user-detail.test.ts`) and are named `*.test.ts`. The WTR config file is `web-test-runner.config.js` and discovers tests with `src/**/test/**/*.test.ts`.

### Required Import Order In Tests

`src/test/setup.ts` must be the very first import in every test file. For tests in component `test/` folders, import it as `../../test/setup`:

```ts
import '../../test/setup'; // MUST be first
import { fixture, html, expect, elementUpdated } from '@open-wc/testing';
import '../my-component';
```

### Why `setup.ts` Must Come First

The `@instance` decorator calls `Container.get()` at class-definition time, when a component module is first evaluated by the browser. If the service is not already registered, Aurelia falls back to `autoRegister()`, which calls `Reflect.getOwnMetadata()` from `reflect-metadata`.

In browser-based tests, relying on `import 'reflect-metadata'` alone is not sufficient because sibling ES module evaluation order is not guaranteed. This can cause `TypeError: Reflect.getOwnMetadata is not a function` during component import.

`src/test/setup.ts` avoids that by importing `reflect-metadata`, bootstrapping the DI container, and pre-registering injectable services with `Container.instance.registerInstance(...)` before any component module is imported. This bypasses `autoRegister()` entirely.

### Adding New Services To Tests

When adding a new service under `src/services/` that is used by tested components, register it in `src/test/setup.ts`:

```ts
import { MyService } from '../services/my-service';

Container.instance.registerInstance(MyService, new MyService());
```

### Writing Tests

Use Mocha (`describe` / `it`) with Chai assertions and `@open-wc/testing` helpers such as `fixture` and `elementUpdated`.

When testing `EventAggregator` behavior, retrieve the shared instance from the container and dispose subscriptions after assertions:

```ts
import { Container } from 'aurelia-dependency-injection';
import { EventAggregator } from 'aurelia-event-aggregator';

const ea = Container.instance.get(EventAggregator);
const sub = ea.subscribe('MY_EVENT', (data) => {
  /* ... */
});

sub.dispose();
```
