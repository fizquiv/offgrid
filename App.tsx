import React, {useEffect, useState} from 'react';
import {
  View,
  Text,
  FlatList,
  PermissionsAndroid,
  Platform,
  Pressable,
  TextInput,
  ActivityIndicator,
  StyleSheet,
} from 'react-native';
import {BleManager, Device} from 'react-native-ble-plx';
import base64 from 'react-native-base64';

import styles from './AppStyles';

const manager = new BleManager();

// Replace with valid Service/Characteristic UUIDs for your device
const SERVICE_UUID = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
const CHARACTERISTIC_UUID = '6e400002-b5a3-f393-e0a9-e50e24dcca9e';

export default function App() {
  const [devices, setDevices] = useState<Device[]>([]);
  const [connectedDevice, setConnectedDevice] = useState<Device | null>(null);
  const [showSendScreen, setShowSendScreen] = useState(false);
  const [message, setMessage] = useState('');
  const [connectingId, setConnectingId] = useState<string | null>(null);

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
    setDevices([]);
    manager.startDeviceScan(null, null, (error, device) => {
      if (error) {
        console.log('Scan error:', error);
        return;
      }

      // Only add devices that have a name
      if (device && (device.name || device.localName)) {
        setDevices(prev => {
          if (prev.some(d => d.id === device.id)) {
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
      setConnectingId(device.id);
      console.log('Connecting to device:', device.id);

      const connected = await manager.connectToDevice(device.id);
      await connected.discoverAllServicesAndCharacteristics();
      setConnectedDevice(connected);
      setShowSendScreen(true);
    } catch (error) {
      console.warn('Connection error:', error);
    } finally {
      setConnectingId(null);
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
      await connectedDevice.writeCharacteristicWithoutResponseForService(
        SERVICE_UUID,
        CHARACTERISTIC_UUID,
        base64Message,
      );
      console.log('Message sent:', message);
      setMessage('');
    } catch (error) {
      console.warn('Write error:', error);
    }
  };

  // --- SCREENS ---

  // 1) Scan screen
  const renderScanScreen = () => {
    return (
      <View style={styles.container}>
        <Text style={styles.headerText}>BLE Device Scanner</Text>

        {/* "Scan" button with press effect */}
        <Pressable
          onPress={startScan}
          style={({pressed}) => [
            styles.primaryButton,
            pressed && {backgroundColor: '#379FFF'},
          ]}>
          <Text style={styles.primaryButtonText}>Scan for BLE Devices</Text>
        </Pressable>

        <FlatList
          data={devices}
          keyExtractor={item => item.id}
          style={{marginTop: 10}}
          renderItem={({item}) => (
            <Pressable
              onPress={() => handleConnectToDevice(item)}
              style={({pressed}) => [
                styles.deviceItem,
                pressed && {backgroundColor: '#2c2c2c'},
              ]}>
              <View style={styles.deviceRow}>
                <View style={{flex: 1}}>
                  <Text style={styles.deviceItemName}>
                    {item.name || item.localName}
                  </Text>
                  <Text style={styles.deviceItemId}>{item.id}</Text>
                </View>
                {item.id === connectingId && (
                  <ActivityIndicator size="small" color="#fff" />
                )}
              </View>
            </Pressable>
          )}
        />
      </View>
    );
  };

  // 2) Send screen
  const renderSendScreen = () => {
    return (
      <View style={styles.container}>
        <Text style={styles.headerText}>Connected</Text>
        <Text style={styles.subHeaderText}>
          {connectedDevice?.name || connectedDevice?.localName}
        </Text>

        <TextInput
          style={styles.input}
          placeholder="Enter message"
          placeholderTextColor="#888"
          value={message}
          onChangeText={setMessage}
        />

        {/* "Send Message" button with press effect */}
        <Pressable
          onPress={sendMessage}
          style={({pressed}) => [
            styles.primaryButton,
            pressed && {backgroundColor: '#379FFF'},
          ]}>
          <Text style={styles.primaryButtonText}>Send Message</Text>
        </Pressable>

        {/* "Disconnect" button with press effect */}
        <Pressable
          onPress={() => {
            connectedDevice?.cancelConnection();
            setConnectedDevice(null);
            setShowSendScreen(false);
          }}
          style={({pressed}) => [
            styles.primaryButton,
            {backgroundColor: '#8B0000', marginTop: 20},
            pressed && {backgroundColor: '#A30000'},
          ]}>
          <Text style={styles.primaryButtonText}>Disconnect</Text>
        </Pressable>
      </View>
    );
  };

  return showSendScreen ? renderSendScreen() : renderScanScreen();
}
