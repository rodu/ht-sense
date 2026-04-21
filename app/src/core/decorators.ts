import { Container } from 'aurelia-dependency-injection';

export interface InstanceDecoratorOptions {
  type: any;
}

export function instance(options: InstanceDecoratorOptions) {
  return (target: any, propertyKey: string) => {
    target[propertyKey] = Container.instance.get(options.type);
  };
}
