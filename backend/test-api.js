// Test script to verify all API endpoints
// Run with: node test-api.js

const baseURL = 'http://localhost:5000';

// Helper function to make requests
async function testEndpoint(method, endpoint, body = null) {
  console.log(`\n🧪 Testing: ${method} ${endpoint}`);
  
  try {
    const options = {
      method,
      headers: { 'Content-Type': 'application/json' }
    };
    
    if (body) {
      options.body = JSON.stringify(body);
    }
    
    const response = await fetch(`${baseURL}${endpoint}`, options);
    const data = await response.json();
    
    console.log(`✅ Status: ${response.status}`);
    console.log('📦 Response:', JSON.stringify(data, null, 2));
    
    return data;
  } catch (error) {
    console.error(`❌ Error: ${error.message}`);
  }
}

// Run all tests
async function runTests() {
  console.log('🚀 Starting API Tests...\n');
  console.log('=' .repeat(60));
  
  // Test 1: Health Check
  await testEndpoint('GET', '/health');
  
  // Test 2: Send Sensor Data
  await testEndpoint('POST', '/api/sensor-data', {
    lane1: 45.5,
    lane2: 120.3,
    lane3: 80.7,
    lane4: 200.0
  });
  
  // Test 3: Get Latest Sensor Data
  await testEndpoint('GET', '/api/sensor-data/latest');
  
  // Test 4: Get Sensor History
  await testEndpoint('GET', '/api/sensor-data?limit=5');
  
  // Test 5: Generate Decision
  await testEndpoint('POST', '/api/decision/generate');
  
  // Test 6: Get Latest Decision
  await testEndpoint('GET', '/api/decision/latest');
  
  // Test 7: Get Decision History
  await testEndpoint('GET', '/api/decisions?limit=5');
  
  console.log('\n' + '='.repeat(60));
  console.log('✅ All tests completed!');
}

// Run the tests
runTests().catch(console.error);