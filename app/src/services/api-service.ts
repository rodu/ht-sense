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

interface SensorReading {
  // A UUID value
  id: string;
  // A unix timestamp
  timestamp: number;
  // value with 2 decimal places
  temperatureCelsius: number;
  // value with 2 decimal places
  temperatureFarenaith: number;
  // value with 2 decimal places
  humidity: number;
}

const sensorData: SensorReading[] = [
  {
    id: 'a1f9e6f0-1c23-4a2d-8e41-9a5b8d0a1001',
    timestamp: 1713705600,
    temperatureCelsius: 21.34,
    temperatureFarenaith: 70.41,
    humidity: 46.12,
  },
  {
    id: 'b2c8d5e1-2d34-4b3e-9f52-aa6c9e1b2002',
    timestamp: 1713705900,
    temperatureCelsius: 21.48,
    temperatureFarenaith: 70.66,
    humidity: 45.87,
  },
  {
    id: 'c3d7c4f2-3e45-4c4f-a063-bb7dae2c3003',
    timestamp: 1713706200,
    temperatureCelsius: 21.55,
    temperatureFarenaith: 70.79,
    humidity: 45.63,
  },
  {
    id: 'd4e6b3a3-4f56-4d5a-b174-cc8ebf3d4004',
    timestamp: 1713706500,
    temperatureCelsius: 21.62,
    temperatureFarenaith: 70.92,
    humidity: 45.41,
  },
  {
    id: 'e5f5a2b4-5067-4e6b-c285-dd9fc04e5005',
    timestamp: 1713706800,
    temperatureCelsius: 21.77,
    temperatureFarenaith: 71.19,
    humidity: 45.08,
  },
  {
    id: 'f6a491c5-6178-4f7c-d396-eea0d15f6006',
    timestamp: 1713707100,
    temperatureCelsius: 21.93,
    temperatureFarenaith: 71.47,
    humidity: 44.76,
  },
  {
    id: '07b380d6-7289-408d-e4a7-ffb1e2707007',
    timestamp: 1713707400,
    temperatureCelsius: 22.05,
    temperatureFarenaith: 71.69,
    humidity: 44.51,
  },
  {
    id: '18c27fe7-839a-419e-f5b8-10c2f3818008',
    timestamp: 1713707700,
    temperatureCelsius: 22.16,
    temperatureFarenaith: 71.89,
    humidity: 44.27,
  },
  {
    id: '29d16ef8-94ab-42af-86c9-21d304929009',
    timestamp: 1713708000,
    temperatureCelsius: 22.24,
    temperatureFarenaith: 72.03,
    humidity: 44.03,
  },
  {
    id: '3ae05d09-a5bc-43b0-97da-32e415a3a010',
    timestamp: 1713708300,
    temperatureCelsius: 22.31,
    temperatureFarenaith: 72.16,
    humidity: 43.78,
  },
  {
    id: '4bdf4c1a-b6cd-44c1-a8eb-43f526b4b011',
    timestamp: 1713708600,
    temperatureCelsius: 22.46,
    temperatureFarenaith: 72.43,
    humidity: 43.52,
  },
  {
    id: '5cce3b2b-c7de-45d2-b9fc-540637c5c012',
    timestamp: 1713708900,
    temperatureCelsius: 22.58,
    temperatureFarenaith: 72.64,
    humidity: 43.26,
  },
  {
    id: '6dbd2a3c-d8ef-46e3-ca0d-651748d6d013',
    timestamp: 1713709200,
    temperatureCelsius: 22.69,
    temperatureFarenaith: 72.84,
    humidity: 43.02,
  },
  {
    id: '7eac194d-e900-47f4-db1e-762859e7e014',
    timestamp: 1713709500,
    temperatureCelsius: 22.75,
    temperatureFarenaith: 72.95,
    humidity: 42.79,
  },
  {
    id: '8f9b085e-fa11-4805-ec2f-87396af8f015',
    timestamp: 1713709800,
    temperatureCelsius: 22.88,
    temperatureFarenaith: 73.18,
    humidity: 42.55,
  },
  {
    id: '908af76f-0b22-4916-fd30-984a7b090016',
    timestamp: 1713710100,
    temperatureCelsius: 22.97,
    temperatureFarenaith: 73.35,
    humidity: 42.28,
  },
  {
    id: 'a179e680-1c33-4a27-8e41-a95b8c1a0017',
    timestamp: 1713710400,
    temperatureCelsius: 23.04,
    temperatureFarenaith: 73.47,
    humidity: 42.01,
  },
  {
    id: 'b268d591-2d44-4b38-9f52-ba6c9d2b0018',
    timestamp: 1713710700,
    temperatureCelsius: 23.12,
    temperatureFarenaith: 73.62,
    humidity: 41.74,
  },
  {
    id: 'c357c4a2-3e55-4c49-a063-cb7dae3c0019',
    timestamp: 1713711000,
    temperatureCelsius: 23.27,
    temperatureFarenaith: 73.89,
    humidity: 41.49,
  },
  {
    id: 'd446b3b3-4f66-4d5a-b174-dc8ebf4d0020',
    timestamp: 1713711300,
    temperatureCelsius: 23.36,
    temperatureFarenaith: 74.05,
    humidity: 41.23,
  },
  {
    id: 'e535a2c4-5077-4e6b-c285-ed9fc05e0021',
    timestamp: 1713711600,
    temperatureCelsius: 23.44,
    temperatureFarenaith: 74.19,
    humidity: 40.97,
  },
  {
    id: 'f62491d5-6188-4f7c-d396-fea0d16f0022',
    timestamp: 1713711900,
    temperatureCelsius: 23.52,
    temperatureFarenaith: 74.34,
    humidity: 40.73,
  },
  {
    id: '071380e6-7299-408d-e4a7-0fb1e2710023',
    timestamp: 1713712200,
    temperatureCelsius: 23.63,
    temperatureFarenaith: 74.53,
    humidity: 40.49,
  },
  {
    id: '18227ff7-83aa-419e-f5b8-20c2f3820024',
    timestamp: 1713712500,
    temperatureCelsius: 23.71,
    temperatureFarenaith: 74.68,
    humidity: 40.22,
  },
  {
    id: '29316e08-94bb-42af-86c9-31d304933025',
    timestamp: 1713712800,
    temperatureCelsius: 23.84,
    temperatureFarenaith: 74.91,
    humidity: 39.98,
  },
];
