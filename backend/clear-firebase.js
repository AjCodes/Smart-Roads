// Run this script to clear old Firebase data
// Usage: node clear-firebase.js

import { db } from './config/firebase.js';

async function clearData() {
  try {
    console.log('🗑️  Clearing old Firebase data...');

    // Clear sensor data
    await db.ref('sensor_data').remove();
    console.log('✅ Cleared sensor_data');

    // Clear decisions
    await db.ref('decisions').remove();
    console.log('✅ Cleared decisions');

    console.log('\n✅ All data cleared! Restart your ESP32 and backend.');
    process.exit(0);
  } catch (error) {
    console.error('❌ Error:', error);
    process.exit(1);
  }
}

clearData();
