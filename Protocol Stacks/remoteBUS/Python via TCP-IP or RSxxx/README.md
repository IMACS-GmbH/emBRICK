# IMACS GmbH - Python emBrick Libary

[![N|emBrick](https://www.embrick.de/wp-content/uploads/2015/02/emBRICK-Logo@2x.png)](https://www.embrick.de/)

## About


## Installation
Run the following command in your terminal:

```
Under Windows: pip install emBRICK
```

```
Under Linux: pip3 install emBRICK
```

## Installation on RaspberryPI without Internet Connection

### Download

[install.sh](https://raw.githubusercontent.com/IMACS-GmbH/emBRICK/main/Protocol%20Stacks/remoteBUS/Python%20via%20TCP-IP%20or%20RSxxx/install.sh)

[emBRICK-0.07-rpi.tgz](https://github.com/IMACS-GmbH/emBRICK/raw/main/Protocol%20Stacks/remoteBUS/Python%20via%20TCP-IP%20or%20RSxxx/emBRICK-0.07-rpi.tgz)

[install_ubuntu.sh](https://github.com/IMACS-GmbH/emBRICK/raw/main/Protocol%20Stacks/remoteBUS/Python%20via%20TCP-IP%20or%20RSxxx/install_ubuntu.sh)

[emBRICK-0.11-ubuntu.tgz](https://github.com/IMACS-GmbH/emBRICK/raw/main/Protocol%20Stacks/remoteBUS/Python%20via%20TCP-IP%20or%20RSxxx/emBRICK-0.11-ubuntu.tgz)

or create emBRICK-0.07-rpi.tgz yourself with Bash Script download.sh

[download.sh](https://github.com/IMACS-GmbH/emBRICK/raw/main/Protocol%20Stacks/remoteBUS/Python%20via%20TCP-IP%20or%20RSxxx/download.sh)
```
wget https://raw.githubusercontent.com/IMACS-GmbH/emBRICK/main/Protocol%20Stacks/remoteBUS/Python%20via%20TCP-IP%20or%20RSxxx/download.sh -O download.sh
sudo bash download.sh
```

### Copy on Target without Internet (Raspberry Pi OS)

Copy emBRICK-0.07-rpi.tgz & install.sh on the Target and start install.sh Bash Script with
```
wget https://raw.githubusercontent.com/IMACS-GmbH/emBRICK/main/Protocol%20Stacks/remoteBUS/Python%20via%20TCP-IP%20or%20RSxxx/install.sh -O install.sh
wget https://github.com/IMACS-GmbH/emBRICK/raw/main/Protocol%20Stacks/remoteBUS/Python%20via%20TCP-IP%20or%20RSxxx/emBRICK-0.07-rpi.tgz  -O emBRICK-0.07-rpi.tgz
sudo bash install.sh
```
### Copy on Target without Internet (Ubuntu Server)
Copy emBRICK-0.07-rpi.tgz & install.sh on the Target and start install.sh Bash Script with
```
wget https://github.com/IMACS-GmbH/emBRICK/raw/main/Protocol%20Stacks/remoteBUS/Python%20via%20TCP-IP%20or%20RSxxx/install_ubuntu.sh -O install_ubuntu.sh
wget https://github.com/IMACS-GmbH/emBRICK/raw/main/Protocol%20Stacks/remoteBUS/Python%20via%20TCP-IP%20or%20RSxxx/emBRICK-0.11-ubuntu.tgz -O emBRICK-0.11-ubuntu.tgz
sudo bash install_ubuntu.sh
```

## Usage

Please read the PD_CouplingBrickStarterkit.pdf Chapter 8 for more Details. [PD_CouplingBrickStarterkit.pdf](https://github.com/IMACS-GmbH/emBRICK/raw/main/Protocol%20Stacks/remoteBUS/_PD_CouplingBrick%20Starterkit.pdf "Heading link")
