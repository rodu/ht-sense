// Import this module FIRST in every test file — before any component imports.
//
// Problem: @instance decorators call Container.get() at class-definition time
// (when a component module is first evaluated). If the service isn't registered,
// Aurelia calls autoRegister() which internally calls Reflect.getOwnMetadata()
// from reflect-metadata. That polyfill may not be loaded yet because ES module
// evaluation order in the browser is not guaranteed for sibling imports.
//
// Fix: pre-register all injectable services using registerInstance() BEFORE
// any component module is evaluated. StrategyResolver(0) returns the provided
// instance directly, bypassing autoRegister() and Reflect.getOwnMetadata entirely.
import 'reflect-metadata';
import '../core/di';
import { Container } from 'aurelia-dependency-injection';
import { EventAggregator } from 'aurelia-event-aggregator';
import { ApiService } from '../services/api-service';

// Pre-register singletons so Container.get() never falls through to autoRegister().
Container.instance.registerInstance(EventAggregator, new EventAggregator());
Container.instance.registerInstance(ApiService, new ApiService());
