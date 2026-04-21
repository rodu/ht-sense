export interface IUser {
  id: string;
  firstName: string;
  lastName: string;
  email: string;
  department: string;
  jobTitle: string;
}

export class ApiService {
  constructor() {
    console.log('ApiService contructor called');
  }

  public async loadUsers(): Promise<IUser[]> {
    return new Promise((resolve) => {
      setTimeout(() => resolve(mockData), 500);
    });
  }
}

const mockData = [
  {
    id: 'd6b4f575-ed3a-45f9-a946-a0b3f8e3f874',
    firstName: 'Abc',
    lastName: 'Def',
    email: 'abcdef0example.com',
    department: 'Engineering',
    jobTitle: 'Senior Financial Analyst',
  },
];
