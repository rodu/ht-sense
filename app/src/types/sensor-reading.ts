export interface SensorReading {
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
