import { LitElement, css, html } from 'lit';
import { customElement, property } from 'lit/decorators.js';
import { IUser } from '../services/api-service';
import { EventAggregator } from 'aurelia-event-aggregator';
import { instance } from '../core/decorators';

@customElement('user-detail')
export class UserDetail extends LitElement {
  @property({ type: Object })
  public user: IUser | null = null;

  @instance({ type: EventAggregator })
  private eventAggregator!: EventAggregator;

  private handleUserClick(event: MouseEvent) {
    event.stopPropagation();

    this.eventAggregator.publish('USER_SELECTED', { ...this.user });
  }

  render() {
    if (!this.user) return;

    console.log('Rendering user:', this.user.id);
    return html`
      <div class="data-item" @click=${this.handleUserClick}>
        <div>
          <strong>Name:</strong>
          <span>${this.user.firstName}</span>
        </div>
        <div>
          <strong>Last name:</strong>
          <span>${this.user.lastName}</span>
        </div>
        <div>
          <strong>Email:</strong>
          <span>${this.user.email}</span>
        </div>
        <div>
          <strong>Department:</strong>
          <span>${this.user.department}</span>
        </div>
        <div>
          <strong>Job Title:</strong>
          <span>${this.user.jobTitle}</span>
        </div>
      </div>
    `;
  }

  static styles = css`
    .data-item {
      text-align: left;
      margin-bottom: 10px;
    }
  `;
}
