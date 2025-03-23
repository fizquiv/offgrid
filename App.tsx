import React, { useEffect, useState } from 'react';
import {
  View,
  Text,
  Button,
  FlatList,
  PermissionsAndroid,
  Platform,
  Pressable,
  TextInput,
} from 'react-native';
import { BleManager, Device } from 'react-native-ble-plx';
import base64 from 'react-native-base64';

const manager = new BleManager();

// Replace with valid Service and Characteristic UUIDs for your device
const SERVICE_UUID = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
const CHARACTERISTIC_UUID = '6e400002-b5a3-f393-e0a9-e50e24dcca9e';

export default function App() {
  const [devices, setDevices] = useState<Device[]>([]);
  const [connectedDevice, setConnectedDevice] = useState<Device | null>(null);
  const [showSendScreen, setShowSendScreen] = useState(false);
  const [message, setMessage] = useState('');

  useEffect(() => {
    if (Platform.OS === 'android') {
      requestPermissions();
    }
  }, []);

  const requestPermissions = async () => {
    const granted = await PermissionsAndroid.requestMultiple([
      PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
      PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
      PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
    ]);
    console.log('Permissions:', granted);
  };

  // Start scanning for BLE devices
  const startScan = () => {
    // Reset the devices list
    setDevices([]);

    manager.startDeviceScan(null, null, (error, device) => {
      if (error) {
        console.log('Scan error:', error);
        return;
      }

      if (device && (device.name || device.localName)) {
        // Add device to state if not already in the list
        setDevices((prev) => {
          if (prev.find((d) => d.id === device.id)) {
            return prev;
          }
          return [...prev, device];
        });
      }
    });

    // Stop scan after 10 seconds
    setTimeout(() => {
      manager.stopDeviceScan();
    }, 10000);
  };

  // Connect to a selected device
  const handleConnectToDevice = async (device: Device) => {
    try {
      console.log('Connecting to device:', device.id);
      const connected = await manager.connectToDevice(device.id);
      console.log('Discovering services and characteristics...');
      await connected.discoverAllServicesAndCharacteristics();
      // Optionally: read or write from known characteristic here to verify
      setConnectedDevice(connected);
      setShowSendScreen(true);
    } catch (error) {
      console.warn('Connection error:', error);
    }
  };

  // Write the user-entered message to the BLE characteristic
  const sendMessage = async () => {
    if (!connectedDevice) {
      console.warn('No device connected.');
      return;
    }
    try {
      const base64Message = base64.encode(message);

      // Use writeCharacteristicWithResponseForService or writeCharacteristicWithoutResponseForService
      await connectedDevice.writeCharacteristicWithResponseForService(
        SERVICE_UUID,
        CHARACTERISTIC_UUID,
        base64Message
      );

      console.log('Message sent:', message);
      setMessage('');
    } catch (error) {
      console.warn('Write error:', error);
    }
  };

  // --- RENDERING UI ---

  // Scan screen: shows "Scan" button + list of devices
  const renderScanScreen = () => {
    return (
      <View style={{ flex: 1, paddingTop: 50, backgroundColor: '#000' }}>
        <Button title="Scan for BLE Devices" onPress={startScan} />
        <FlatList
          data={devices}
          keyExtractor={(item) => item.id}
          renderItem={({ item }) => (
            <Pressable
              onPress={() => handleConnectToDevice(item)}
              style={{ padding: 10, borderBottomWidth: 1, borderBottomColor: '#ccc' }}
            >
              <Text style={{ color: 'white', fontSize: 16 }}>
                {item.name || item.localName}
              </Text>
              <Text style={{ color: 'white', fontSize: 12 }}>{item.id}</Text>
            </Pressable>
          )}
        />
      </View>
    );
  };

  // Send screen: once connected, show a text input + button
  const renderSendScreen = () => {
    return (
      <View style={{ flex: 1, paddingTop: 50, backgroundColor: '#000', padding: 20 }}>
        <Text style={{ color: 'white', marginBottom: 10 }}>
          Connected to {connectedDevice?.name || connectedDevice?.localName || 'Unknown'}
        </Text>

        <TextInput
          style={{
            backgroundColor: '#fff',
            marginBottom: 10,
            padding: 10,
            borderRadius: 4,
          }}
          placeholder="Enter message"
          value={message}
          onChangeText={setMessage}
        />

        <Button title="Send Message" onPress={sendMessage} />

        <View style={{ height: 20 }} />
        <Button
          title="Disconnect"
          onPress={() => {
            connectedDevice?.cancelConnection();
            setConnectedDevice(null);
            setShowSendScreen(false);
          }}
        />
      </View>
    );
  };

  // Conditionally render the scan screen or the send screen
  return showSendScreen ? renderSendScreen() : renderScanScreen();
}
