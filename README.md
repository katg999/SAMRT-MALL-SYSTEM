Kasamba Mall Smart System
The Kasamba Mall Smart System is designed to transform Kasamba Mall into a smart, secure, and efficient environment using IoT technologies. The system aims to enhance the experience of tenants, management, and customers by automating access control, managing tenant and customer facilities, and monitoring mall usage. The system incorporates features like occupancy tracking, access restrictions, and remote management via a smartphone app with Bluetooth capabilities.

Features

1. Access Control and Occupancy Monitoring
   Entrances and Exits: The mall has two entrances and one exit. Sensors at each entrance and exit count the number of people entering or leaving the building, ensuring a controlled flow and secure access.
   Escalator System: The mall is equipped with escalators for moving between floors. Each escalator has a sensor that detects and counts individuals as they ascend. Escalators move in one direction only.
   Smart Washrooms: Washroom access is restricted and managed by a passcode system. Each tenant receives a unique access code to enter the washrooms. Tenants can also generate temporary access codes for their clients, which expire after a preset time limit.
   Occupancy Status: When a washroom is occupied, a message is displayed on the entry panel, restricting further access until the washroom is available.
2. Tenant Rent and Dues Management
   Rent Configuration: Rent is based on floor location, as follows:
   Ground Floor: Base rent fee.
   Second Floor: Rent is set to three-quarters of the base fee.
   Third Floor: Rent is three-quarters of the second floor’s fee.
   Due Payment and Access Restriction: Tenants who have overdue rent lose access to the washrooms, along with their clients.
   Fee Update: Management can update fees via a serial console interface.
3. Management and Smartphone App Control
   Management can control various aspects of the system through a smartphone application with Bluetooth connectivity. Available controls include:

Escalator Control: Enable or disable escalators, encouraging stair use when escalators are disabled.
Account Disabling: Restrict access to tenants with overdue payments.
Washroom Access Override: Restrict access in emergencies.
Real-Time Occupancy Monitoring: View current occupancy levels of different floors and facilities.
Access Code Expiration Control: Adjust expiration settings for tenant and customer washroom access codes.
Security Override: Allow temporary access to restricted areas during emergencies.
Technical Specifications

1. Hardware Components
   Sensors: Installed at entrances, exits, and escalators to detect and count foot traffic.
   Bluetooth Module: Enables smartphone connectivity for remote control of system features.
   Microcontroller: Interfaces with sensors and manages data, sends/receives commands, and controls connected components.
   Display Units: Placed outside washrooms to show occupancy and access messages.
2. Software Components
   Access Control Logic: Implements entry and exit restrictions, escalator direction logic, and washroom access control.
   Tenant Rent Management: Tracks rent payments, restrictions, and updates in real-time.
   Serial Console Interface: Allows management to configure rent fees, access codes, and tenant restrictions.
   Bluetooth Smartphone Application: A user-friendly mobile app that allows management to control escalators, access codes, tenant accounts, and monitor occupancy.
3. Bluetooth Functionality (to be implemented)
   The Bluetooth functionality in the Kasamba Mall Smart System will enable management to interact with the following features:

Control escalators and restrict access during specific times.
Disable access to tenants with unpaid dues.
Adjust passcode expiration time for tenants’ clients.
Monitor occupancy levels in real time.
Temporarily override access restrictions during emergencies.
Installation and Setup
Hardware Setup
Install the required sensors at entrances, exits, and escalators.
Set up the microcontroller to handle input and output connections for the sensors, display units, and Bluetooth module.
Software Installation
Code Deployment: Upload the control code from Atmel Studio to the microcontroller.
Serial Console Configuration: Connect to the serial console interface to configure rent settings and tenant access.
Smartphone App Setup: Download the app on a smartphone with Bluetooth capability to allow remote management access.
System Usage
Occupancy Counting: The system automatically counts people as they enter, exit, or use escalators.
Washroom Access: Tenants and authorized clients can enter the washroom using their passcodes.
Rent Payment Updates: Tenants can update their payments, allowing the system to restore access permissions.
Management Console and App Control: Management can use the app or serial console to control system settings, disable access, or enable escalator restrictions as needed.
Future Enhancements
Integration with Cloud Monitoring: To view occupancy and access history remotely.
Voice-Controlled Commands: Allow management to issue voice commands to control aspects of the system.
Expanded Tenant Features: Enable tenants to access real-time updates on rent status or view access codes they’ve assigned.
Energy Management: Integrate energy-saving modes for escalators and lighting based on mall occupancy levels.
Facial Recognition Access: Use facial recognition as an additional or alternative access method for tenants and customers.
License
This project is licensed under the MIT License.

Contact
For more information, reach out to me on email katex911@gmail.com.

This README provides a complete overview of the Kasamba Mall Smart System, helping users and developers understand its functionality and purpose. Good luck with your project implementation!
