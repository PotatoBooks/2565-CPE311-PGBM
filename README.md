# Solar Tracking with STM32l152RB

## Requirement
สร้างระบบ Solar tracking ให้กับแผงโซลาร์เซลล์ โดยใช้บอร์ด STM32L152RB เป็นตัวควบคุม

## Gantt Chart
![gantt-chart](https://github.com/PotatoBooks/2565-CPE311-PGBM/blob/main/gantt-chart.png)

## AGILE Methodology Model
![agile](https://github.com/PotatoBooks/2565-CPE311-PGBM/blob/main/agile-methodology.png)

### Sprint 1
#### Plan
* Develope SG-90 Servo Motor Movement
* Get 2 ADC values from 2 LDRs Correctly
#### Reflaction
* SG-90 Servo Motor move perfectly 
* Get 2 ADC values from 2 LDRs correctly

### Sprint 2
#### Plan
* Integrate the system
#### Reflaction
* 2 ADC values from 2 LDRs are not correct
* Need to Adjust Servo Motor's angle
* Integrated the system

### Sprin 3
#### Plan
* Fix ADC Problem
* Testing and Debugging 
#### Reflaction
* Fixed ADC problem
* system work according to requirement

## Model Design

### Components
* 2x LDR Sensors
* 2x 10K ohm Resistor
* 1x SG-90 Servo Motor
* 1x Solar Panel
* 1x STM32l152

### Block Diagram
![block-diagram](https://github.com/PotatoBooks/2565-CPE311-PGBM/blob/main/block-diagram.png)

### Pin Allocation
![pin-allo](https://github.com/PotatoBooks/2565-CPE311-PGBM/blob/main/pin-allowcation.png)

### Circuit Diagram
![circuit-diagram](https://github.com/PotatoBooks/2565-CPE311-PGBM/blob/main/CircuitDiagram.png)

### Flowchart
![flowchart](https://github.com/PotatoBooks/2565-CPE311-PGBM/blob/main/Solar%20Tracking%20Flowchart.jpg)
