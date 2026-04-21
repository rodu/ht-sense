import '../test/setup'; // bootstrap DI before any component import
import { fixture, html, expect, elementUpdated } from '@open-wc/testing';
import { EventAggregator } from 'aurelia-event-aggregator';
import { Container } from 'aurelia-dependency-injection';
import './user-detail';
import type { UserDetail } from './user-detail';
import type { IUser } from '../services/api-service';

const mockUser: IUser = {
  id: 'test-1',
  firstName: 'John',
  lastName: 'Doe',
  email: 'john.doe@example.com',
  department: 'Engineering',
  jobTitle: 'Developer',
};

describe('UserDetail', () => {
  it('renders nothing when user is null', async () => {
    const el = await fixture<UserDetail>(html`<user-detail></user-detail>`);
    expect(el.shadowRoot!.querySelector('.data-item')).to.be.null;
  });

  it('renders user data when a user is provided', async () => {
    const el = await fixture<UserDetail>(
      html`<user-detail .user=${mockUser}></user-detail>`
    );
    const item = el.shadowRoot!.querySelector('.data-item');
    expect(item).to.not.be.null;
    expect(item!.textContent).to.include('John');
    expect(item!.textContent).to.include('Doe');
    expect(item!.textContent).to.include('john.doe@example.com');
    expect(item!.textContent).to.include('Engineering');
    expect(item!.textContent).to.include('Developer');
  });

  it('re-renders when the user property changes', async () => {
    const el = await fixture<UserDetail>(
      html`<user-detail .user=${mockUser}></user-detail>`
    );
    el.user = {
      ...mockUser,
      firstName: 'Jane',
    };
    await elementUpdated(el);
    expect(el.shadowRoot!.querySelector('.data-item')!.textContent).to.include(
      'Jane'
    );
  });

  it('publishes USER_SELECTED via EventAggregator on click', async () => {
    const ea = Container.instance.get(EventAggregator);
    let received: IUser | null = null;
    const sub = ea.subscribe('USER_SELECTED', (data: IUser) => {
      received = data;
    });

    const el = await fixture<UserDetail>(
      html`<user-detail .user=${mockUser}></user-detail>`
    );
    el.shadowRoot!.querySelector<HTMLElement>('.data-item')!.click();

    expect(received).to.deep.equal(mockUser);
    sub.dispose();
  });
});
