# NDN Mobility Modifications - Thesis Research

This repository contains my modifications to ndnSIM for PhD research on Context-Aware Routing Model for NDN-based VANET.

## ⚠ Important Note
This is a **modified fork** of the original ndnSIM. It includes custom changes for mobility research.

## Original Repository
- Base: https://github.com/named-data-ndnSIM/ndnSIM
- Original authors: Named Data Networking team

## My Modifications

### 1. Context-Aware Forwarding
- Location: `ns-3/src/ndnSIM/NFD/daemon/fw/`
- Enhanced for vehicular network scenarios
- Context-aware Interest & Data forwarding

### 2. Mobiloity-Aware Routing
- Location: `ns-3/src/ndnSIM/NFD/daemon/fw`


### 3. Mobility Models Integration
- Location: `ns-3/src/ndnSIM/helper/`
- Integration with ns-3 mobility models
- Support for OpenStreetMap-based mobility

## Building
```bash
cd ns-3
./waf configure
./waf
```

## Running Examples
```bash
cd ns-3
./waf --run scratch/your-mobility-scenario
```

## Thesis Reference
This work is part of my PhD thesis in Electronics and Computer Engineering at University of Minho.

## Contact
- Elídio Silva
- Email: id6644@alunos.uminho.pt
- GitHub: @6Nokturnal6
