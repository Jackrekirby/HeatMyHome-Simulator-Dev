import argparse
import sys
import matplotlib.pyplot as plt
import pgeocode
import os
import csv
import math
import time

print("Hello, welcome to the home heating calculator")
print(f'working directory: {os.getcwd()}')
if os.getcwd().endswith('src'):
    os.chdir("..")

START_TIME = time.time()

# command line args: python src/heat_ninja_cml.py 2 "CV4 7AL" 3000 60.0 0.5 20.0 0

# create parser
parser = argparse.ArgumentParser()
# add arguments to the parser
parser.add_argument("num_occupants", type=int)
parser.add_argument("postcode", type=str)
parser.add_argument("epc_space_heating", type=int)
parser.add_argument("house_size", type=float)
parser.add_argument("tes_max_vol", type=float)
parser.add_argument("temp", type=float)
parser.add_argument("debug", type=int)
# debug = 0: no file out, 1: optimal out, 2: all out
# parse the arguments
args = parser.parse_args()
print("Arguments:", args)

# EPC Info
"""https://www.scottishepcregister.org.uk/CustomerFacingPortal/TermsAndConditions
https://find-energy-certificate.digital.communities.gov.uk/find-a-certificate/search-by-postcode"""
# INPUTS
Occupants_Num = args.num_occupants  # Avg 2
Location_Input = args.postcode  # TR1 2ES, BH14 0EY, CV4 7AL, M1 4HF, FY8 2JG, IV1 1SN, HS2 9AG, needs to be capitals
EPC_Space_Heating = args.epc_space_heating  # Annual space heating in kWh on EPC, LE10 9330
House_Size = args.house_size  # Avg 87, small 60, LE10 85
TES_Volume_Maximum = args.tes_max_vol  # m3 minimum 0.1, recommended max 3.0
Temp = args.temp  # Thermostat set point

# CONSTANTS
Hot_Water_Temp = 51
Grid_Emissions = 212  # Current UK 212gCO2e/kWh electricity
# https://www.gov.uk/government/publications/greenhouse-gas-reporting-conversion-factors-2021

Temp_Profile = [Temp-2, Temp-2, Temp-2, Temp-2, Temp-2, Temp-2, Temp-2, Temp, Temp, Temp, Temp, Temp, Temp, Temp,
                Temp, Temp, Temp, Temp, Temp, Temp, Temp, Temp, Temp-2, Temp-2]
HP_Temp_Profile = [Temp, Temp, Temp, Temp, Temp, Temp, Temp, Temp, Temp, Temp, Temp, Temp, Temp, Temp, Temp, Temp,
                   Temp, Temp, Temp, Temp, Temp, Temp, Temp, Temp]

nomi = pgeocode.Nominatim("GB")  # GB code includes Northern Ireland
Location = nomi.query_postal_code(Location_Input)
Latitude = Location.latitude
Longitude = Location.longitude

# DHW DWELLING CONSTANTS
# Hourly ratios and temperature source "Measurement of Domestic Hot Water Consumption in Dwellings"
DHW_Hourly_Ratios = (0.025, 0.018, 0.011, 0.010, 0.008, 0.013, 0.017, 0.044, 0.088, 0.075, 0.060, 0.056, 0.050,
                     0.043, 0.036, 0.029, 0.030, 0.036, 0.053, 0.074, 0.071, 0.059, 0.050, 0.041)
if Latitude < 52.2:  # South of England
    Cold_Water_Temp = (12.1, 11.4, 12.3, 15.2, 16.1, 19.3, 21.2, 20.1, 19.5, 16.8, 13.7, 12.4)
elif Latitude < 53.3:  # Middle of England and Wales
    Cold_Water_Temp = (12.9, 13.3, 14.4, 16.3, 17.7, 19.7, 21.8, 20.1, 20.3, 17.8, 15.3, 14.0)
elif Latitude < 54.95:  # North of England and Northern Ireland
    Cold_Water_Temp = (9.6, 9.3, 10.7, 13.7, 15.3, 17.3, 19.3, 18.6, 17.9, 15.5, 12.3, 10.5)
else:  # Scotland
    Cold_Water_Temp = (9.6, 9.2, 9.8, 13.2, 14.5, 16.8, 19.4, 18.5, 17.5, 15.1, 13.7, 12.4)
DHW_Monthly_Factor = (1.10, 1.06, 1.02, 0.98, 0.94, 0.90, 0.90, 0.94, 0.98, 1.02, 1.06, 1.10)  # SAP plus below values
Showers_Vol = (0.45 * Occupants_Num + 0.65) * 28.8  # Litres, 28.8 equivalent of Mixer with TES
Bath_Vol = (0.13 * Occupants_Num + 0.19) * 50.8  # Assumes shower is present
Other_Vol = 9.8 * Occupants_Num + 14
DHW_Avg_Daily_Vol = Showers_Vol + Bath_Vol + Other_Vol

# SPACE HEATING DWELLING CONSTANTS
Heat_Capacity = (250 * House_Size) / 3600  # kWh/K, 250 kJ/m2K average UK dwelling specific heat capacity in SAP
Body_Heat_Gain = (Occupants_Num * 60) / 1000  # kWh
PF_sg = math.sin(math.pi / 180 * 90 / 2)  # Assume windows are vertical, so no in roof windows
Asg_s = (-0.66 * PF_sg ** 3) + (-0.106 * PF_sg ** 2) + (2.93 * PF_sg)
Bsg_s = (3.63 * PF_sg ** 3) + (-0.374 * PF_sg ** 2) + (-7.4 * PF_sg)
Csg_s = (-2.71 * PF_sg ** 3) + (-0.991 * PF_sg ** 2) + (4.59 * PF_sg) + 1
Asg_n = (26.3 * PF_sg ** 3) + (-38.5 * PF_sg ** 2) + (14.8 * PF_sg)
Bsg_n = (-16.5 * PF_sg ** 3) + (27.3 * PF_sg ** 2) + (-11.9 * PF_sg)
Csg_n = (-1.06 * PF_sg ** 3) + (-0.0872 * PF_sg ** 2) + (-0.191 * PF_sg) + 1
Solar_Declination = (-20.7, -12.8, -1.8, 9.8, 18.8, 23.1, 21.2, 13.7, 2.9, -8.7, -18.4, -23.0)  # Monthly values

# EPC iterative calculations for Thermal Transmittance
EPC_Occupants_Num = 1 + 1.76 * (1 - math.exp(-0.000349 * (House_Size - 13.9)**2)) + 0.0013 * (House_Size - 13.9)
EPC_Body_Gain = (EPC_Occupants_Num * 60) / 1000  # kWh

if Location_Input[0:2] == "AL" or Location_Input[0:4] == "CM21" or Location_Input[0:4] == "CM22" or\
        Location_Input[0:4] == "CM23" or Location_Input[0:2] == "CR" or Location_Input[0:2] == "EC" or\
        Location_Input[0:2] == "HA" or Location_Input[0:2] == "HP" or Location_Input[0:2] == "KT" or\
        Location_Input[0:2] == "LU" or Location_Input[0:2] == "MK" or Location_Input[0:2] == "NW" or\
        Location_Input[0:2] == "OX" or Location_Input[0:2] == "SE" or Location_Input[0:2] == "SG" or\
        Location_Input[0:2] == "SL" or Location_Input[0:2] == "SM" or Location_Input[0:3] == "SN7" or\
        Location_Input[0:2] == "SW" or Location_Input[0:2] == "TW" or Location_Input[0:2] == "UB" or\
        Location_Input[0:2] == "WC" or Location_Input[0:2] == "WD":
    EPC_Outside_Temp = [5.1, 5.6, 7.4, 9.9, 13.0, 16.0, 17.9, 17.8, 15.2, 11.6, 8.0, 5.1]  # Region 1
    EPC_Solar_Irradiance = [30, 56, 98, 157, 195, 217, 203, 173, 127, 73, 39, 24]  # Thames
elif Location_Input[0:2] == "BN" or Location_Input[0:2] == "BR" or Location_Input[0:2] == "CT" or\
        Location_Input[0:2] == "DA" or Location_Input[0:4] == "GU28" or Location_Input[0:4] == "GU29" or\
        Location_Input[0:2] == "ME" or Location_Input[0:4] == "PO18" or Location_Input[0:4] == "PO19" or\
        Location_Input[0:4] == "PO20" or Location_Input[0:4] == "PO21" or Location_Input[0:4] == "PO22" or\
        Location_Input[0:4] == "RH10" or Location_Input[0:4] == "RH11" or Location_Input[0:4] == "RH12" or\
        Location_Input[0:4] == "RH13" or Location_Input[0:4] == "RH14" or Location_Input[0:4] == "RH15" or\
        Location_Input[0:4] == "RH16" or Location_Input[0:4] == "RH17" or Location_Input[0:4] == "RH18" or\
        Location_Input[0:4] == "RH19" or Location_Input[0:4] == "RH20" or Location_Input[0:2] == "TN":
    EPC_Outside_Temp = [5.0, 5.4, 7.1, 9.5, 12.6, 15.4, 17.4, 17.5, 15.0, 11.7, 8.1, 5.2]  # Region 2
    EPC_Solar_Irradiance = [32, 59, 104, 170, 208, 231, 216, 182, 133, 77, 41, 25]  # South East England
elif Location_Input[0:2] == "BH" or Location_Input[0:2] == "DT" or Location_Input[0:4] == "GU11" or\
        Location_Input[0:4] == "GU12" or Location_Input[0:4] == "GU14" or Location_Input[0:4] == "GU30" or\
        Location_Input[0:4] == "GU31" or Location_Input[0:4] == "GU32" or Location_Input[0:4] == "GU33" or\
        Location_Input[0:4] == "GU34" or Location_Input[0:4] == "GU35" or Location_Input[0:4] == "GU46" or\
        Location_Input[0:4] == "GU51" or Location_Input[0:4] == "GU52" or Location_Input[0:2] == "PO" or\
        Location_Input[0:4] == "RG21" or Location_Input[0:4] == "RG22" or Location_Input[0:4] == "RG23" or\
        Location_Input[0:4] == "RG24" or Location_Input[0:4] == "RG25" or Location_Input[0:4] == "RG26" or\
        Location_Input[0:4] == "RG27" or Location_Input[0:4] == "RG28" or Location_Input[0:4] == "RG29" or\
        Location_Input[0:2] == "SO" or Location_Input[0:3] == "SP6" or Location_Input[0:3] == "SP7" or\
        Location_Input[0:3] == "SP8" or Location_Input[0:3] == "SP9" or Location_Input[0:4] == "SP10" or\
        Location_Input[0:4] == "SP11":
    EPC_Outside_Temp = [5.4, 5.7, 7.3, 9.6, 12.6, 15.4, 17.3, 17.3, 15.0, 11.8, 8.4, 5.5]  # Region 3
    EPC_Solar_Irradiance = [35, 62, 109, 172, 209, 235, 217, 185, 138, 80, 44, 27]  # Southern England
elif Location_Input[0:2] == "EX" or Location_Input[0:2] == "PL" or Location_Input[0:2] == "TQ" or\
        Location_Input[0:2] == "TR":
    EPC_Outside_Temp = [6.1, 6.4, 7.5, 9.3, 11.9, 14.5, 16.2, 16.3, 14.6, 11.8, 9.0, 6.4]  # Region 4
    EPC_Solar_Irradiance = [36, 63, 111, 174, 210, 233, 204, 182, 136, 78, 44, 28]  # South West England
elif Location_Input[0:2] == "BA" or Location_Input[0:2] == "BS" or Location_Input[0:2] == "CF" or\
        Location_Input[0:2] == "GL" or Location_Input[0:2] == "TA":
    EPC_Outside_Temp = [4.9, 5.3, 7.0, 9.3, 12.2, 15.0, 16.7, 16.7, 14.4, 11.1, 7.8, 4.9]  # Region 5
    EPC_Solar_Irradiance = [32, 59, 105, 167, 201, 226, 206, 175, 130, 74, 40, 25]  # Severn England and Wales
elif Location_Input[0:2] == "CV" or Location_Input[0:2] == "DE" or Location_Input[0:2] == "DY" or\
        Location_Input[0:2] == "HR" or Location_Input[0:2] == "LE" or Location_Input[0:2] == "NN" or\
        Location_Input[0:3] == "S18" or Location_Input[0:3] == "S32" or Location_Input[0:3] == "S33" or\
        Location_Input[0:3] == "S40" or Location_Input[0:3] == "S41" or Location_Input[0:3] == "S42" or\
        Location_Input[0:3] == "S43" or Location_Input[0:3] == "S44" or Location_Input[0:3] == "S45" or\
        Location_Input[0:4] == "SK13" or Location_Input[0:4] == "SK17" or Location_Input[0:4] == "SK22" or\
        Location_Input[0:4] == "SK23" or Location_Input[0:2] == "ST" or Location_Input[0:2] == "TF" or\
        Location_Input[0:2] == "WR" or Location_Input[0:2] == "WS" or Location_Input[0:2] == "WV":
    EPC_Outside_Temp = [4.3, 4.8, 6.6, 9.0, 11.8, 14.8, 16.6, 16.5, 14.0, 10.5, 7.1, 4.2]  # Region 6
    EPC_Solar_Irradiance = [28, 55, 97, 153, 191, 208, 194, 163, 121, 69, 35, 23]  # Midlands
elif Location_Input[0:2] == "BB" or Location_Input[0:2] == "BL" or Location_Input[0:2] == "CH" or\
        Location_Input[0:2] == "CW" or Location_Input[0:2] == "FY" or Location_Input[0:2] == "OL" or\
        Location_Input[0:2] == "PR" or Location_Input[0:4] == "SY14" or Location_Input[0:2] == "WA" or\
        Location_Input[0:2] == "WN":
    EPC_Outside_Temp = [4.7, 5.2, 6.7, 9.1, 12.0, 14.7, 16.4, 16.3, 14.1, 10.7, 7.5, 4.6]  # Region 7
    EPC_Solar_Irradiance = [24, 51, 95, 152, 191, 203, 186, 152, 115, 65, 31, 20]  # West Pennines England and Wales
elif Location_Input[0:2] == "CA" or Location_Input[0:2] == "DG" or Location_Input[0:3] == "LA7" or\
        Location_Input[0:3] == "LA8" or Location_Input[0:3] == "LA9" or Location_Input[0:4] == "LA10" or\
        Location_Input[0:4] == "LA11" or Location_Input[0:4] == "LA12" or Location_Input[0:4] == "LA13" or\
        Location_Input[0:4] == "LA14" or Location_Input[0:4] == "LA15" or Location_Input[0:4] == "LA16" or\
        Location_Input[0:4] == "LA17" or Location_Input[0:4] == "LA18" or Location_Input[0:4] == "LA19" or\
        Location_Input[0:4] == "LA20" or Location_Input[0:4] == "LA21" or Location_Input[0:4] == "LA22" or\
        Location_Input[0:4] == "LA23":
    EPC_Outside_Temp = [3.9, 4.3, 5.6, 7.9, 10.7, 13.2, 14.9, 14.8, 12.8, 9.7, 6.6, 3.7]  # Region 8
    EPC_Solar_Irradiance = [23, 51, 95, 157, 200, 203, 194, 156, 113, 62, 30, 19]  # NW England and SW Scotland
elif Location_Input[0:3] == "DH4" or Location_Input[0:3] == "DH5" or Location_Input[0:4] == "EH43" or\
        Location_Input[0:4] == "EH44" or Location_Input[0:4] == "EH45" or Location_Input[0:4] == "EH46" or\
        Location_Input[0:2] == "NE" or Location_Input[0:2] == "TD":
    EPC_Outside_Temp = [4.0, 4.5, 5.8, 7.9, 10.4, 13.3, 15.2, 15.1, 13.1, 9.7, 6.6, 3.7]  # Region 9
    EPC_Solar_Irradiance = [23, 50, 92, 151, 200, 196, 187, 153, 11, 61, 30, 18]  # Boarders
elif Location_Input[0:4] == "BD23" or Location_Input[0:4] == "BD24" or Location_Input[0:2] == "DH" or\
        Location_Input[0:2] == "DL" or Location_Input[0:2] == "HG" or Location_Input[0:4] == "LS24" or\
        Location_Input[0:3] == "SR7" or Location_Input[0:3] == "SR8" or Location_Input[0:2] == "TS":
    EPC_Outside_Temp = [4.0, 4.6, 6.1, 8.3, 10.9, 13.8, 15.8, 15.6, 13.5, 10.1, 6.7, 3.8]  # Region 10
    EPC_Solar_Irradiance = [25, 51, 95, 152, 196, 198, 190, 156, 115, 64, 32, 20]  # North East England
elif Location_Input[0:2] == "BD" or Location_Input[0:2] == "DN" or Location_Input[0:2] == "HD" or\
        Location_Input[0:2] == "HU" or Location_Input[0:2] == "HX" or Location_Input[0:2] == "LN" or\
        Location_Input[0:2] == "LS" or Location_Input[0:2] == "NG" or Location_Input[0:3] == "PE9" or\
        Location_Input[0:4] == "PE10" or Location_Input[0:4] == "PE11" or Location_Input[0:4] == "PE12" or\
        Location_Input[0:4] == "PE20" or Location_Input[0:4] == "PE21" or Location_Input[0:4] == "PE22" or\
        Location_Input[0:4] == "PE23" or Location_Input[0:4] == "PE24" or Location_Input[0:4] == "PE25" or\
        Location_Input[0:2] == "WF" or Location_Input[0:4] == "YO15" or Location_Input[0:4] == "YO16" or\
        Location_Input[0:4] == "YO25":
    EPC_Outside_Temp = [4.3, 4.9, 6.5, 8.9, 11.7, 14.6, 16.6, 16.4, 14.1, 10.6, 7.1, 4.2]  # Region 11
    EPC_Solar_Irradiance = [26, 54, 96, 150, 192, 200, 189, 157, 115, 66, 33, 21]  # East Pennines
elif Location_Input[0:2] == "CB" or Location_Input[0:2] == "CM" or Location_Input[0:2] == "CO" or\
        Location_Input[0:3] == "EN9" or Location_Input[0:2] == "IG" or Location_Input[0:2] == "IP" or\
        Location_Input[0:2] == "NR" or Location_Input[0:2] == "PE" or Location_Input[0:2] == "RM" or\
        Location_Input[0:2] == "SS":
    EPC_Outside_Temp = [4.7, 5.2, 7.0, 9.5, 12.5, 15.4, 17.6, 17.6, 15.0, 11.4, 7.7, 4.7]  # Region 12
    EPC_Solar_Irradiance = [30, 58, 101, 165, 203, 220, 206, 173, 128, 74, 39, 24]  # East Anglia
elif Location_Input[0:2] == "LD" or Location_Input[0:4] == "LL23" or Location_Input[0:4] == "LL24" or\
        Location_Input[0:4] == "LL25" or Location_Input[0:4] == "LL26" or Location_Input[0:4] == "LL27" or\
        Location_Input[0:4] == "LL30" or Location_Input[0:4] == "LL31" or Location_Input[0:4] == "LL32" or\
        Location_Input[0:4] == "LL33" or Location_Input[0:4] == "LL34" or Location_Input[0:4] == "LL35" or\
        Location_Input[0:4] == "LL36" or Location_Input[0:4] == "LL37" or Location_Input[0:4] == "LL38" or\
        Location_Input[0:4] == "LL39" or Location_Input[0:4] == "LL40" or Location_Input[0:4] == "LL41" or\
        Location_Input[0:4] == "LL42" or Location_Input[0:4] == "LL43" or Location_Input[0:4] == "LL44" or\
        Location_Input[0:4] == "LL45" or Location_Input[0:4] == "LL46" or Location_Input[0:4] == "LL47" or\
        Location_Input[0:4] == "LL48" or Location_Input[0:4] == "LL49" or Location_Input[0:4] == "LL50" or\
        Location_Input[0:4] == "LL51" or Location_Input[0:4] == "LL52" or Location_Input[0:4] == "LL53" or\
        Location_Input[0:4] == "LL54" or Location_Input[0:4] == "LL55" or Location_Input[0:4] == "LL56" or\
        Location_Input[0:4] == "LL57" or Location_Input[0:4] == "LL58" or Location_Input[0:4] == "LL59" or\
        Location_Input[0:4] == "LL60" or Location_Input[0:4] == "LL61" or Location_Input[0:4] == "LL62" or\
        Location_Input[0:4] == "LL63" or Location_Input[0:4] == "LL64" or Location_Input[0:4] == "LL65" or\
        Location_Input[0:4] == "LL66" or Location_Input[0:4] == "LL67" or Location_Input[0:4] == "LL68" or\
        Location_Input[0:4] == "LL69" or Location_Input[0:4] == "LL70" or Location_Input[0:4] == "LL71" or\
        Location_Input[0:4] == "LL72" or Location_Input[0:4] == "LL73" or Location_Input[0:4] == "LL74" or\
        Location_Input[0:4] == "LL75" or Location_Input[0:4] == "LL76" or Location_Input[0:4] == "LL77" or\
        Location_Input[0:4] == "LL78" or Location_Input[0:3] == "NP8" or Location_Input[0:4] == "SA14" or\
        Location_Input[0:4] == "SA15" or Location_Input[0:4] == "SA16" or Location_Input[0:4] == "SA17" or\
        Location_Input[0:4] == "SA18" or Location_Input[0:4] == "SA19" or Location_Input[0:4] == "SA20" or\
        Location_Input[0:4] == "SA31" or Location_Input[0:4] == "SA32" or Location_Input[0:4] == "SA33" or\
        Location_Input[0:4] == "SA34" or Location_Input[0:4] == "SA35" or Location_Input[0:4] == "SA36" or\
        Location_Input[0:4] == "SA37" or Location_Input[0:4] == "SA38" or Location_Input[0:4] == "SA39" or\
        Location_Input[0:4] == "SA40" or Location_Input[0:4] == "SA41" or Location_Input[0:4] == "SA42" or\
        Location_Input[0:4] == "SA43" or Location_Input[0:4] == "SA44" or Location_Input[0:4] == "SA45" or\
        Location_Input[0:4] == "SA46" or Location_Input[0:4] == "SA47" or Location_Input[0:4] == "SA48" or\
        Location_Input[0:4] == "SA61" or Location_Input[0:4] == "SA62" or Location_Input[0:4] == "SA63" or\
        Location_Input[0:4] == "SA64" or Location_Input[0:4] == "SA65" or Location_Input[0:4] == "SA66" or\
        Location_Input[0:4] == "SA67" or Location_Input[0:4] == "SA68" or Location_Input[0:4] == "SA69" or\
        Location_Input[0:4] == "SA70" or Location_Input[0:4] == "SA71" or Location_Input[0:4] == "SA72" or\
        Location_Input[0:4] == "SA73" or Location_Input[0:4] == "SY15" or Location_Input[0:4] == "SY16" or\
        Location_Input[0:4] == "SY17" or Location_Input[0:4] == "SY18" or Location_Input[0:4] == "SY19" or\
        Location_Input[0:4] == "SY20" or Location_Input[0:4] == "SY21" or Location_Input[0:4] == "SY22" or\
        Location_Input[0:4] == "SY23" or Location_Input[0:4] == "SY24" or Location_Input[0:4] == "SY25":
    EPC_Outside_Temp = [5.0, 5.3, 6.5, 8.5, 11.2, 13.7, 15.3, 15.3, 13.5, 10.7, 7.8, 5.2]  # Region 13
    EPC_Solar_Irradiance = [29, 57, 104, 164, 205, 220, 199, 167, 120, 68, 35, 22]  # Wales
elif Location_Input[0:2] == "FK" or Location_Input[0:2] == "KA" or Location_Input[0:2] == "ML" or\
        Location_Input[0:2] == "PA" or Location_Input[0:4] == "PH49" or Location_Input[0:4] == "PH50":
    EPC_Outside_Temp = [4.0, 4.4, 5.6, 7.9, 10.4, 13.0, 14.5, 14.4, 12.5, 9.3, 6.5, 3.8]  # Region 14
    EPC_Solar_Irradiance = [19, 46, 88, 148, 196, 193, 185, 150, 101, 55, 25, 15]  # West Scotland
elif Location_Input[0:2] == "DD" or Location_Input[0:2] == "EH" or Location_Input[0:2] == "KY":
    EPC_Outside_Temp = [3.6, 4.0, 5.4, 7.7, 10.1, 12.9, 14.6, 14.5, 12.5, 9.2, 6.1, 3.2]  # Region 15
    EPC_Solar_Irradiance = [21, 46, 89, 146, 198, 191, 183, 150, 106, 57, 27, 15]  # East Scotland
elif Location_Input[0:2] == "AB" or Location_Input[0:4] == "IV30" or Location_Input[0:4] == "IV31" or\
        Location_Input[0:4] == "IV32" or Location_Input[0:4] == "IV36" or Location_Input[0:4] == "PH26":
    EPC_Outside_Temp = [3.3, 3.6, 5.0, 7.1, 9.3, 12.2, 14.0, 13.9, 12.0, 8.8, 5.7, 2.9]  # Region 16
    EPC_Solar_Irradiance = [19, 45, 89, 143, 194, 188, 177, 144, 101, 54, 25, 14]  # North East Scotland
elif Location_Input[0:2] == "IV" or Location_Input[0:4] == "PH19" or Location_Input[0:4] == "PH20" or\
        Location_Input[0:4] == "PH21" or Location_Input[0:4] == "PH22" or Location_Input[0:4] == "PH23" or\
        Location_Input[0:4] == "PH24" or Location_Input[0:4] == "PH25" or Location_Input[0:4] == "PH30" or\
        Location_Input[0:4] == "PH31" or Location_Input[0:4] == "PH32" or Location_Input[0:4] == "PH33" or\
        Location_Input[0:4] == "PH34" or Location_Input[0:4] == "PH35" or Location_Input[0:4] == "PH36" or\
        Location_Input[0:4] == "PH37" or Location_Input[0:4] == "PH38" or Location_Input[0:4] == "PH39" or\
        Location_Input[0:4] == "PH40" or Location_Input[0:4] == "PH41" or Location_Input[0:4] == "PH42" or\
        Location_Input[0:4] == "PH43" or Location_Input[0:4] == "PH44":
    EPC_Outside_Temp = [3.1, 3.2, 4.4, 6.6, 8.9, 11.4, 13.2, 13.1, 11.3, 8.2, 5.4, 2.7]  # Region 17
    EPC_Solar_Irradiance = [17, 43, 85, 145, 189, 185, 170, 139, 98, 51, 22, 12]  # Highlands
elif Location_Input[0:2] == "HS":
    EPC_Outside_Temp = [5.2, 5.0, 5.8, 7.6, 9.7, 11.8, 13.4, 13.6, 12.1, 9.6, 7.3, 5.2]  # Region 18
    EPC_Solar_Irradiance = [16, 41, 87, 155, 205, 206, 185, 148, 101, 51, 21, 11]  # Western Isles
elif Location_Input[0:4] == "KW15" or Location_Input[0:4] == "KW16" or Location_Input[0:4] == "KW17":
    EPC_Outside_Temp = [4.4, 4.2, 5.0, 7.0, 8.9, 11.2, 13.1, 13.2, 11.7, 9.1, 6.6, 4.3]  # Region 19
    EPC_Solar_Irradiance = [14, 39, 84, 143, 205, 201, 178, 145, 100, 50, 19, 9]  # Orkney
elif Location_Input[0:2] == "ZE":
    EPC_Outside_Temp = [4.6, 4.1, 4.7, 6.5, 8.3, 10.5, 12.4, 12.8, 11.4, 8.8, 6.5, 4.6]  # Region 20
    EPC_Solar_Irradiance = [12, 34, 79, 135, 196, 190, 168, 144, 90, 46, 16, 7]  # Shetland
elif Location_Input[0:2] == "BT":
    EPC_Outside_Temp = [4.8, 5.2, 6.4, 10.9, 13.5, 15.0, 14.9, 13.1, 10.0, 7.2, 4.7]  # Region 21
    EPC_Solar_Irradiance = [24, 52, 96, 155, 201, 198, 183, 150, 107, 61, 30, 18]  # Northern Ireland

# Need to be after other options
elif Location_Input[0:2] == "NP" or Location_Input[0:2] == "SA" or Location_Input[0:2] == "SN" or\
        Location_Input[0:2] == "SP":
    EPC_Outside_Temp = [4.9, 5.3, 7.0, 9.3, 12.2, 15.0, 16.7, 16.7, 14.4, 11.1, 7.8, 4.9]  # Region 5 "NP" before "N"
    EPC_Solar_Irradiance = [32, 59, 105, 167, 201, 226, 206, 175, 130, 74, 40, 25]  # Severn England and Wales
elif Location_Input[0:1] == "E" or Location_Input[0:2] == "EN" or Location_Input[0:2] == "GU" or\
        Location_Input[0:1] == "N" or Location_Input[0:2] == "RG" or Location_Input[0:2] == "RH" or\
        Location_Input[0:1] == "W":
    EPC_Outside_Temp = [5.1, 5.6, 7.4, 9.9, 13.0, 16.0, 17.9, 17.8, 15.2, 11.6, 8.0, 5.1]  # Region 1
    EPC_Solar_Irradiance = [30, 56, 98, 157, 195, 217, 203, 173, 127, 73, 39, 24]  # Thames
elif Location_Input[0:1] == "B" or Location_Input[0:2] == "SY":
    EPC_Outside_Temp = [4.3, 4.8, 6.6, 9.0, 11.8, 14.8, 16.6, 16.5, 14.0, 10.5, 7.1, 4.2]  # Region 6
    EPC_Solar_Irradiance = [28, 55, 97, 153, 191, 208, 194, 163, 121, 69, 35, 23]  # Midlands
elif Location_Input[0:1] == "L" or Location_Input[0:2] == "LA" or Location_Input[0:2] == "LL" or\
        Location_Input[0:1] == "M" or Location_Input[0:2] == "SK":
    EPC_Outside_Temp = [4.7, 5.2, 6.7, 9.1, 12.0, 14.7, 16.4, 16.3, 14.1, 10.7, 7.5, 4.6]  # Region 7
    EPC_Solar_Irradiance = [24, 51, 95, 152, 191, 203, 186, 152, 115, 65, 31, 20]  # West Pennines England and Wales
elif Location_Input[0:2] == "SR":
    EPC_Outside_Temp = [4.0, 4.5, 5.8, 7.9, 10.4, 13.3, 15.2, 15.1, 13.1, 9.7, 6.6, 3.7]  # Region 9
    EPC_Solar_Irradiance = [23, 50, 92, 151, 200, 196, 187, 153, 11, 61, 30, 18]  # Boarders
elif Location_Input[0:2] == "YO":
    EPC_Outside_Temp = [4.0, 4.6, 6.1, 8.3, 10.9, 13.8, 15.8, 15.6, 13.5, 10.1, 6.7, 3.8]  # Region 10
    EPC_Solar_Irradiance = [25, 51, 95, 152, 196, 198, 190, 156, 115, 64, 32, 20]  # North East England
elif Location_Input[0:1] == "S":
    EPC_Outside_Temp = [4.3, 4.9, 6.5, 8.9, 11.7, 14.6, 16.6, 16.4, 14.1, 10.6, 7.1, 4.2]  # Region 11
    EPC_Solar_Irradiance = [26, 54, 96, 150, 192, 200, 189, 157, 115, 66, 33, 21]  # East Pennines
elif Location_Input[0:1] == "G":
    EPC_Outside_Temp = [4.0, 4.4, 5.6, 7.9, 10.4, 13.0, 14.5, 14.4, 12.5, 9.3, 6.5, 3.8]  # Region 14
    EPC_Solar_Irradiance = [19, 46, 88, 148, 196, 193, 185, 150, 101, 55, 25, 15]  # West Scotland
elif Location_Input[0:2] == "PH":
    EPC_Outside_Temp = [3.6, 4.0, 5.4, 7.7, 10.1, 12.9, 14.6, 14.5, 12.5, 9.2, 6.1, 3.2]  # Region 15
    EPC_Solar_Irradiance = [21, 46, 89, 146, 198, 191, 183, 150, 106, 57, 27, 15]  # East Scotland
elif Location_Input[0:2] == "KW":
    EPC_Outside_Temp = [3.1, 3.2, 4.4, 6.6, 8.9, 11.4, 13.2, 13.1, 11.3, 8.2, 5.4, 2.7]  # Region 17
    EPC_Solar_Irradiance = [17, 43, 85, 145, 189, 185, 170, 139, 98, 51, 22, 12]  # Highlands
else:
    print("Postcode region can not be found")
    exit()


# EPC CALCULATIONS
def function_epc_day_calculation():
    global EPC_Day
    if m == 5 or m == 6 or m == 7 or m == 8:  # NO HEATING Jun, Jul, Aug, Sep, -1 for count
        epc_temp_profile = [7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7]
    elif EPC_Day % 7 == 0 or (EPC_Day + 1) % 7 == 0:  # Weekend, not summer
        epc_temp_profile = [7, 7, 7, 7, 7, 7, 7, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20]
    else:  # Weekday, not summer
        epc_temp_profile = [7, 7, 7, 7, 7, 7, 7, 20, 20, 20, 7, 7, 7, 7, 7, 7, 20, 20, 20, 20, 20, 20, 20, 20]
    for h in range(24):
        global Inside_Temp_Current
        global EPC_Demand
        desired_temp_current = epc_temp_profile[h]
        outside_temp_current = EPC_Outside_Temp[m]
        solar_irradiance_current = EPC_Solar_Irradiance[m]
        solar_declination_current = Solar_Declination[m]
        solar_height_factor = math.cos(math.pi / 180 * (Latitude - solar_declination_current))
        ratio_sg_south = Asg_s * solar_height_factor ** 2 + Bsg_s * solar_height_factor + Csg_s
        ratio_sg_north = Asg_n * solar_height_factor ** 2 + Bsg_n * solar_height_factor + Csg_n
        incident_irradiance_sg_s = solar_irradiance_current * ratio_sg_south
        incident_irradiance_sg_n = solar_irradiance_current * ratio_sg_north
        solar_gain_s = incident_irradiance_sg_s * (House_Size * 0.15 / 2) * 0.77 * 0.7 * 0.76 * 0.9 / 1000  # kW
        solar_gain_n = incident_irradiance_sg_n * (House_Size * 0.15 / 2) * 0.77 * 0.7 * 0.76 * 0.9 / 1000  # kW

        heat_flow_out = (House_Size * Thermal_Transmittance_Current * (Inside_Temp_Current - outside_temp_current)) /\
            1000
        # heat_flow_out in kWh, +ve means heat flows out of building, -ve heat flows into building
        Inside_Temp_Current += ((- heat_flow_out + solar_gain_s + solar_gain_n + EPC_Body_Gain) /
            Heat_Capacity)
        if Inside_Temp_Current > desired_temp_current:  # Warm enough already, NO heating required
            space_hr_demand = 0
        else:  # Requires heating
            space_hr_demand = (desired_temp_current - Inside_Temp_Current) * Heat_Capacity
            Inside_Temp_Current = desired_temp_current
        EPC_Demand += (space_hr_demand / 0.9)
    EPC_Day += 1


Thermal_Transmittance_Current = 0.5
Thermal_Transmittance = 0.5
Optimised_EPC_Demand = 0
for Iteration in range(251):  # 0.5 to 3.0
    EPC_Day = 1
    Inside_Temp_Current = 20  # Initial temperature
    EPC_Demand = 0
    for m in range(12):  # EPC LOOP
        if m == 3 or m == 5 or m == 8 or m == 10:  # Months with 30 days -1, as for loop starts at 0
            for d in range(30):
                function_epc_day_calculation()
        elif m == 1:
            for d in range(28):
                function_epc_day_calculation()
        else:
            for d in range(31):
                function_epc_day_calculation()
    if ((EPC_Space_Heating - EPC_Demand) ** 2) ** 0.5 < ((EPC_Space_Heating - Optimised_EPC_Demand) ** 2) ** 0.5:
        Optimised_EPC_Demand = EPC_Demand
        Thermal_Transmittance = Thermal_Transmittance_Current
    Thermal_Transmittance_Current += 0.01

print("Dwelling thermal transmittance", "%.2f" % Thermal_Transmittance)
print("Resultant EPC demand", "%.0f" % Optimised_EPC_Demand, "kWh")

Lat_Rounded = str(round(Latitude * 2) / 2)
Lon_Rounded = str(round(Longitude * 2) / 2)
File_Name = "ninja_weather_" + Lat_Rounded + "000_" + Lon_Rounded + "000_uncorrected.csv"
Outside_Temp = []
Solar_Irradiance = []
Coldest_Outside_Temp = 5  # Initial set point, then reduced depending on location weather
if os.path.isfile("data/" + File_Name):
    Weather_File = open("data/" + File_Name, "r")
    Weather_Data = csv.reader(Weather_File)
    for Row in Weather_Data:
        Outside_Temp.append(float(Row[2]))
        Solar_Irradiance.append(float(Row[3]))
        if (float(Row[2])) < Coldest_Outside_Temp:
            Coldest_Outside_Temp = (float(Row[2]))
    Weather_File.close()
else:
    print("Cannot find weather file for that location")
    exit()

Agile_Tariff = []  # Octopus Agile tariff 2020
Agile_File = open("data/Agile Tariff.csv")
Agile_Data = csv.reader(Agile_File)
for Row in Agile_Data:
    Agile_Tariff.append(float(Row[1]))
Agile_File.close()


# DEMAND CALCULATIONS
def function_demand_day_calculation():
    for h in range(24):
        global Inside_Temp_Current
        global Weather_Count
        global Demand_Total
        global DHW_Total

        # DHW HOUR CALCULATIONS
        dhw_mf_current = DHW_Monthly_Factor[m]
        dhw_hr_current = DHW_Hourly_Ratios[h]
        cwt_current = Cold_Water_Temp[m]
        dhw_hr_demand = (DHW_Avg_Daily_Vol * 4.18 * (Hot_Water_Temp - cwt_current) / 3600) * dhw_mf_current * \
            dhw_hr_current

        # SPACE HEATING HOUR CALCULATIONS
        desired_temp_current = Desired_Temp[h]
        outside_temp_current = Outside_Temp[Weather_Count]
        solar_irradiance_current = Solar_Irradiance[Weather_Count]
        solar_declination_current = Solar_Declination[m]
        solar_height_factor = math.cos(math.pi / 180 * (Latitude - solar_declination_current))
        ratio_sg_south = Asg_s * solar_height_factor ** 2 + Bsg_s * solar_height_factor + Csg_s
        ratio_sg_north = Asg_n * solar_height_factor ** 2 + Bsg_n * solar_height_factor + Csg_n
        incident_irradiance_sg_s = solar_irradiance_current * ratio_sg_south
        incident_irradiance_sg_n = solar_irradiance_current * ratio_sg_north
        solar_gain_s = incident_irradiance_sg_s * (House_Size * 0.15 / 2) * 0.77 * 0.7 * 0.76 * 0.9 / 1000  # kW
        solar_gain_n = incident_irradiance_sg_n * (House_Size * 0.15 / 2) * 0.77 * 0.7 * 0.76 * 0.9 / 1000  # kW

        heat_loss = (House_Size * Thermal_Transmittance * (Inside_Temp_Current - outside_temp_current)) / 1000
        # heat_loss in kWh, +ve means heat flows out of building, -ve heat flows into building
        Inside_Temp_Current += ((- heat_loss + solar_gain_s + solar_gain_n + Body_Heat_Gain) / Heat_Capacity)

        if Inside_Temp_Current > desired_temp_current:  # Warm enough already, NO heating required
            space_hr_demand = 0
        else:  # Requires heating
            space_hr_demand = (desired_temp_current - Inside_Temp_Current) * Heat_Capacity
            Inside_Temp_Current = desired_temp_current

        Weather_Count += 1
        Demand_Record.append(dhw_hr_demand + space_hr_demand)
        Demand_Total += (dhw_hr_demand + space_hr_demand)
        DHW_Total += dhw_hr_demand


if args.debug == 3 or args.debug == 4:
    demand_path = f'debug_data/demand_{Occupants_Num}_{Location_Input}_{EPC_Space_Heating}_{House_Size}_{TES_Volume_Maximum}_{Temp}_.csv'
    f_demand = open(demand_path, 'w')

# DEMAND LOOP
HP_Max_Demand = 0
Boiler_Max_Demand = 0
HP_Demand_Total = 0
Boiler_Demand_Total = 0
for p in range(2):
    if p == 0:  # HP demand
        Desired_Temp = HP_Temp_Profile
    else:  # Boilers / electric heating demand
        Desired_Temp = Temp_Profile
    Inside_Temp_Current = Temp
    Weather_Count = 0
    Demand_Record = []
    Demand_Total = 0
    DHW_Total = 0
    for m in range(12):
        if m == 3 or m == 5 or m == 8 or m == 10:  # Months with 30 days -1, as for loop starts at 0
            for d in range(30):
                function_demand_day_calculation()
        elif m == 1:
            for d in range(28):
                function_demand_day_calculation()
        else:
            for d in range(31):
                function_demand_day_calculation()

    if args.debug == 3 or args.debug == 4:
        f_demand.write(f'{DHW_Total:.0f}, {(Demand_Total - DHW_Total):.0f}, {Demand_Total:.0f}, {max(Demand_Record):.1f}\n')

    if p == 0:  # HP
        HP_Demand_Total = Demand_Total
        # water demand, space demand, total demand, peak hour demand
        print("DHW demand:", "%.0f" % DHW_Total, "kWh")
        print("HP space demand:", "%.0f" % (Demand_Total - DHW_Total), "kWh")
        print("HP yearly total thermal demand:", "%.0f" % HP_Demand_Total, "kWh")
        HP_Max_Demand = max(Demand_Record)
        print("HP peak hour demand:", "%.1f" % HP_Max_Demand, "kWh")

    else:  # Anything other than HP
        Boiler_Demand_Total = Demand_Total
        print("DHW demand:", "%.0f" % DHW_Total, "kWh")
        print("Boiler space demand:", "%.0f" % (Demand_Total - DHW_Total), "kWh")
        print("Boiler yearly total thermal demand:", "%.0f" % Boiler_Demand_Total, "kWh")
        Boiler_Max_Demand = max(Demand_Record)
        print("Boiler peak hour demand:", "%.1f" % Boiler_Max_Demand, "kWh")

f_demand.close()

# HEATING CALCULATIONS
def function_heater_day_calculation():
    for h in range(24):
        global Inside_Temp_Current
        global Weather_Count
        global TES_State_of_Charge
        global Operational_Costs_Peak
        global Operational_Costs_Off_Peak
        global Solar_Thermal_Generation_Current
        global Solar_Thermal_Generation_Total
        global Operation_Emissions

        # DHW HOUR CALCULATIONS
        dhw_mf_current = DHW_Monthly_Factor[m]
        dhw_hr_current = DHW_Hourly_Ratios[h]
        cwt_current = Cold_Water_Temp[m]
        dhw_hr_demand = (DHW_Avg_Daily_Vol * 4.18 * (Hot_Water_Temp - cwt_current) / 3600) * dhw_mf_current * \
            dhw_hr_current

        # SPACE HEATING HOUR CALCULATIONS
        if HP_Option == 1 or HP_Option == 2:  # ASHP or GSHP
            desired_min_temp_current = HP_Temp_Profile[h]
        else:  # Electrical heating
            desired_min_temp_current = Temp_Profile[h]
        outside_temp_current = Outside_Temp[Weather_Count]
        solar_irradiance_current = Solar_Irradiance[Weather_Count]
        solar_declination_current = Solar_Declination[m]
        solar_height_factor = math.cos(math.pi / 180 * (Latitude - solar_declination_current))
        agile_tariff_current = Agile_Tariff[Weather_Count]

        ratio_sg_south = Asg_s * solar_height_factor ** 2 + Bsg_s * solar_height_factor + Csg_s
        ratio_sg_north = Asg_n * solar_height_factor ** 2 + Bsg_n * solar_height_factor + Csg_n
        incident_irradiance_sg_s = solar_irradiance_current * ratio_sg_south
        incident_irradiance_sg_n = solar_irradiance_current * ratio_sg_north
        solar_gain_s = incident_irradiance_sg_s * (House_Size * 0.15 / 2) * 0.77 * 0.7 * 0.76 * 0.9 / 1000  # kW
        solar_gain_n = incident_irradiance_sg_n * (House_Size * 0.15 / 2) * 0.77 * 0.7 * 0.76 * 0.9 / 1000  # kW
        # Does not account for angle of the sun across the day
        ratio_roof_south = A_roof_s * solar_height_factor ** 2 + B_roof_s * solar_height_factor + C_roof_s
        incident_irradiance_roof_s = solar_irradiance_current * ratio_roof_south / 1000  # kW/m2

        heat_loss = (House_Size * Thermal_Transmittance * (Inside_Temp_Current - outside_temp_current)) / 1000
        # heat_loss in kWh, +ve means heat flows out of building, -ve heat flows into building
        Inside_Temp_Current += ((- heat_loss + solar_gain_s + solar_gain_n + Body_Heat_Gain) / Heat_Capacity)

        if TES_State_of_Charge <= TES_Charge_Full:  # Currently at nominal temperature ranges
            tes_upper_temperature = 51
            tes_lower_temperature = cwt_current  # Bottom of the tank would still be at CWT,
            tes_thermocline_height = TES_State_of_Charge / TES_Charge_Full  # %, from top down, .25 is top 25%
        elif TES_State_of_Charge <= TES_Charge_Boost:  # At boosted temperature ranges
            tes_upper_temperature = 60
            tes_lower_temperature = 51
            tes_thermocline_height = (TES_State_of_Charge - TES_Charge_Full) / (TES_Charge_Boost - TES_Charge_Full)
        else:  # At max tes temperature
            tes_upper_temperature = 95
            tes_lower_temperature = 60
            tes_thermocline_height = (TES_State_of_Charge - TES_Charge_Boost) / (TES_Charge_Max - TES_Charge_Boost)
        if tes_thermocline_height > 1:
            tes_thermocline_height = 1
        if tes_thermocline_height < 0:
            tes_thermocline_height = 0
        tes_upper_losses = (tes_upper_temperature - Inside_Temp_Current) * U_Value * (math.pi * TES_Radius * 2 *
            (tes_thermocline_height * TES_Radius * 2) + math.pi * TES_Radius ** 2) / 1000  # losses in kWh
        tes_lower_losses = (tes_lower_temperature - Inside_Temp_Current) * U_Value * (math.pi * TES_Radius * 2 *
            ((1 - tes_thermocline_height) * TES_Radius * 2) + math.pi * TES_Radius ** 2) / 1000
        TES_State_of_Charge += - (tes_upper_losses + tes_lower_losses)
        Inside_Temp_Current += (tes_upper_losses + tes_lower_losses) / Heat_Capacity  # TES inside house

        if HP_Option == 0:  # Electric Heater
            cop_current = cop_boost = 1
        elif HP_Option == 1:  # ASHP, source A review of domestic heat pumps
            cop_current = 6.81 - 0.121 * (Hot_Water_Temp - outside_temp_current) + \
                          0.00063 * (Hot_Water_Temp - outside_temp_current) ** 2
            cop_boost = 6.81 - 0.121 * (60 - outside_temp_current) + \
                          0.00063 * (60 - outside_temp_current) ** 2
        else:  # GSHP, source A review of domestic heat pumps
            cop_current = 8.77 - 0.150 * (Hot_Water_Temp - Ground_Temperature) + \
                          0.000734 * (Hot_Water_Temp - Ground_Temperature) ** 2
            cop_boost = 8.77 - 0.150 * (60 - Ground_Temperature) + \
                          0.000734 * (60 - Ground_Temperature) ** 2

        if Solar_Option == 6:  # PVT
            pv_efficiency = (14.7 * (1 - 0.0045 * ((tes_upper_temperature + tes_lower_temperature) / 2 - 25))) / 100
            # https://www.sciencedirect.com/science/article/pii/S0306261919313443#b0175
        else:
            pv_efficiency = 0.1928  # Technology Library https://zenodo.org/record/4692649#.YQEbio5KjIV
            # monocrystalline used for domestic
        pv_generation_current = PV_Size * pv_efficiency * incident_irradiance_roof_s * 0.8  # 80% shading factor

        if Solar_Option >= 2:
            solar_thermal_collector_temperature = (tes_upper_temperature + tes_lower_temperature) / 2
            # Collector to heat from tes lower temperature to tes upper temperature, so use the average temperature
            if incident_irradiance_roof_s == 0:
                Solar_Thermal_Generation_Current = 0
            elif Solar_Option == 2 or Solar_Option == 4:  # Flat plate
                # https://www.sciencedirect.com/science/article/pii/B9781782422136000023
                Solar_Thermal_Generation_Current = Solar_Thermal_Size * (0.78 * incident_irradiance_roof_s -
                    0.0035 * (solar_thermal_collector_temperature - outside_temp_current) -
                    0.000038 * (solar_thermal_collector_temperature - outside_temp_current) ** 2) * 0.8
            elif Solar_Option == 6:  # PVT https://www.sciencedirect.com/science/article/pii/S0306261919313443#b0175
                Solar_Thermal_Generation_Current = Solar_Thermal_Size * (0.726 * incident_irradiance_roof_s -
                    0.003325 * (solar_thermal_collector_temperature - outside_temp_current) -
                    0.0000176 * (solar_thermal_collector_temperature - outside_temp_current) ** 2) * 0.8
            else:  # Evacuated tube https://www.sciencedirect.com/science/article/pii/B9781782422136000023
                Solar_Thermal_Generation_Current = Solar_Thermal_Size * (0.625 * incident_irradiance_roof_s -
                    0.0009 * (solar_thermal_collector_temperature - outside_temp_current) -
                    0.00002 * (solar_thermal_collector_temperature - outside_temp_current) ** 2) * 0.8
            if Solar_Thermal_Generation_Current < 0:
                Solar_Thermal_Generation_Current = 0
            TES_State_of_Charge += Solar_Thermal_Generation_Current
            if TES_State_of_Charge > TES_Charge_Max:  # Dumps any excess solar generated heat to prevent boiling TES
                TES_State_of_Charge = TES_Charge_Max

        # Determines space hour demand and inside temperature
        if Inside_Temp_Current > desired_min_temp_current:
            space_hr_demand = 0
        else:  # Requires space heating
            space_hr_demand = (desired_min_temp_current - Inside_Temp_Current) * Heat_Capacity
            if (space_hr_demand + dhw_hr_demand) < (TES_State_of_Charge + HP_Electrical_Power * cop_current):
                Inside_Temp_Current = desired_min_temp_current
            else:  # Not capable of meeting hourly demand
                if TES_State_of_Charge > 0:
                    space_hr_demand = (TES_State_of_Charge + HP_Electrical_Power * cop_current) - dhw_hr_demand
                else:  # Priority to space demand over TES charging
                    space_hr_demand = (HP_Electrical_Power * cop_current) - dhw_hr_demand
                Inside_Temp_Current += space_hr_demand / Heat_Capacity

        # Determines electrical demand for space and dhw demands
        if (space_hr_demand + dhw_hr_demand) < TES_State_of_Charge:  # TES can provide all demand
            TES_State_of_Charge = TES_State_of_Charge - (space_hr_demand + dhw_hr_demand)
            electrical_demand_current = 0
        elif (space_hr_demand + dhw_hr_demand) < (TES_State_of_Charge + HP_Electrical_Power * cop_current):
            if TES_State_of_Charge > 0:
                electrical_demand_current = (space_hr_demand + dhw_hr_demand - TES_State_of_Charge) / cop_current
                TES_State_of_Charge = 0  # TES needs support so taken to empty if it had any charge
            else:
                electrical_demand_current = (space_hr_demand + dhw_hr_demand) / cop_current
        else:  # TES and HP can't meet hour demand
            electrical_demand_current = HP_Electrical_Power
            if TES_State_of_Charge > 0:
                TES_State_of_Charge = 0

        # Charges TES at off peak electricity times
        if ((Tariff == 0 and 12 < h < 16) or (Tariff == 1 and (h == 23 or h < 6)) or (Tariff == 2 and 12 < h < 16)
                or (Tariff == 3 and 0 <= h < 5) or (Tariff == 4 and agile_tariff_current < 9.0)) \
                and TES_Volume_Current > 0 and TES_State_of_Charge < TES_Charge_Full:
            # Flat rate and smart tariff charges TES at typical day peak air temperature times
            # GSHP is not affected so can keep to these times too
            if (TES_Charge_Full - TES_State_of_Charge) < \
                    ((HP_Electrical_Power - electrical_demand_current) * cop_current):  # Small top up
                electrical_demand_current += (TES_Charge_Full - TES_State_of_Charge) / cop_current
                TES_State_of_Charge = TES_Charge_Full
            else:  # HP can not fully top up in one hour
                TES_State_of_Charge += (HP_Electrical_Power - electrical_demand_current) * cop_current
                electrical_demand_current = HP_Electrical_Power
        pv_remaining_current = pv_generation_current - electrical_demand_current

        # Boost temperature if any spare PV generated electricity, as reduced cop, raises to nominal temp above first
        if pv_remaining_current > 0 and TES_State_of_Charge < TES_Charge_Boost:
            if ((TES_Charge_Boost - TES_State_of_Charge) < (pv_remaining_current * cop_boost)) and\
                    ((TES_Charge_Boost - TES_State_of_Charge) < ((HP_Electrical_Power -
                    electrical_demand_current) * cop_boost)):
                electrical_demand_current += (TES_Charge_Boost - TES_State_of_Charge) / cop_boost
                TES_State_of_Charge = TES_Charge_Boost
            else:
                if pv_remaining_current < HP_Electrical_Power:
                    TES_State_of_Charge += pv_remaining_current * cop_boost
                    electrical_demand_current += pv_remaining_current
                else:
                    TES_State_of_Charge += (HP_Electrical_Power - electrical_demand_current) * cop_boost
                    electrical_demand_current = HP_Electrical_Power

        if TES_State_of_Charge < TES_Charge_Min:  # Take back up to 10L capacity if possible no matter what time
            if (TES_Charge_Min - TES_State_of_Charge) < (HP_Electrical_Power - electrical_demand_current) * \
                    cop_current:
                electrical_demand_current += (TES_Charge_Min - TES_State_of_Charge) / cop_current
                TES_State_of_Charge = TES_Charge_Min
            elif electrical_demand_current < HP_Electrical_Power:  # Can't take all the way back up to 10L charge
                TES_State_of_Charge += (HP_Electrical_Power - electrical_demand_current) * cop_current

        electrical_import = electrical_demand_current - pv_generation_current
        if electrical_import < 0:  # Generating more electricity than using
            pv_equivalent_revenue = - electrical_import
            electrical_import = 0
        else:
            pv_equivalent_revenue = 0

        # Operational costs summation
        if Tariff == 0:  # Flat rate tariff https://www.nimblefins.co.uk/average-cost-electricity-kwh-uk#:~:text=
            # (second part of address) Unit%20Cost%20of%20Electricity%20per,more%20than%20the%20UK%20average
            # Average solar rate https://www.greenmatch.co.uk/solar-energy/solar-panels/solar-panel-grants
            Operational_Costs_Peak += 0.163 * electrical_import - pv_equivalent_revenue * (0.163 + 0.035) / 2
        elif Tariff == 1:  # Economy 7 tariff, same source as flat rate above
            if h == 23 or h < 6:  # Off Peak
                Operational_Costs_Off_Peak += 0.095 * electrical_import - pv_equivalent_revenue * (0.095 + 0.035) / 2
            else:  # Peak
                Operational_Costs_Peak += 0.199 * electrical_import - pv_equivalent_revenue * (0.199 + 0.035) / 2
        elif Tariff == 2:  # Bulb smart, for East Midlands values 2021
            # https://help.bulb.co.uk/hc/en-us/articles/360017795731-About-Bulb-s-smart-tariff
            if 15 < h < 19:  # Peak winter times throughout the year
                Operational_Costs_Peak += 0.2529 * electrical_import - pv_equivalent_revenue * (0.2529 + 0.035) / 2
            else:  # Off peak
                Operational_Costs_Off_Peak += 0.1279 * electrical_import - pv_equivalent_revenue * (0.1279 + 0.035) / 2
        elif Tariff == 3:  # Octopus Go EV, LE10 0YE 2012, https://octopus.energy/go/rates/
            # https://www.octopusreferral.link/octopus-energy-go-tariff/
            if 0 <= h < 5:  # Off Peak
                Operational_Costs_Off_Peak += 0.05 * electrical_import - pv_equivalent_revenue * (0.05 + 0.03) / 2
            else:  # Peak
                Operational_Costs_Peak += 0.1533 * electrical_import - pv_equivalent_revenue * (0.1533 + 0.03) / 2
        else:  # Octopus Agile file 2020
            # 2021 Octopus export rates https://octopus.energy/outgoing/
            if agile_tariff_current < 9.0:  # Off peak, lower range of variable costs
                Operational_Costs_Off_Peak += (agile_tariff_current / 100) * electrical_import - \
                    pv_equivalent_revenue * ((agile_tariff_current/100) + 0.055) / 2
            else:  # Peak, upper range of variable costs
                Operational_Costs_Peak += (agile_tariff_current / 100) * electrical_import - \
                    pv_equivalent_revenue * ((agile_tariff_current/100) + 0.055) / 2

        # Operational emissions summation
        operational_emissions_current = Solar_Thermal_Generation_Current * 22.5  # 22.5 average ST
        # from https://post.parliament.uk/research-briefings/post-pn-0523/
        if PV_Size > 0:
            operational_emissions_current += (pv_generation_current - pv_equivalent_revenue) * 75 + \
                pv_equivalent_revenue * (75 - Grid_Emissions)
        # https://www.parliament.uk/globalassets/documents/post/postpn_383-carbon-footprint-electricity-generation.pdf
        # 75 for PV, 75 - Grid_Emissions show emissions saved for the grid or for reducing other electrical bills
        operational_emissions_current += electrical_import * Grid_Emissions
        Operation_Emissions += operational_emissions_current

        Solar_Thermal_Generation_Total += Solar_Thermal_Generation_Current
        Weather_Count += 1


# HEATER & TES SETTINGS
# print("Electrified heating options at annual costs:")
TES_Volume_Current = 0
Solar_Thermal_Generation_Current = 0
PV_Generation_Current = 0
CapEx = 0
NPC_Years = 20
Discount_Rate = 1.035  # 3.5% standard for UK HMRC
# https://www.finance-ni.gov.uk/articles/step-eight-calculate-net-present-values-and-assess-uncertainties
Current_TES_and_Tariff_Specifications = []
Optimum_TES_and_Tariff_Specifications = []

Ground_Temperature = 15 - (Latitude - 50) * (4 / 9)  # Linear regression ground temp across UK at 100m depth
# Wei's paper and sources
if TES_Volume_Maximum < 0.1:
    TES_Volume_Maximum = 0.1
if TES_Volume_Maximum > 3.0:
    TES_Volume_Maximum = 3.0
TES_Range = TES_Volume_Maximum / 0.1
Solar_Maximum = (int((House_Size / 4) / 2)) * 2  # Quarter of the roof for solar, even number
# Does not account for angle of the sun across the day
PF_roof = math.sin(math.pi / 180 * 35 / 2)  # Assume roof is 35° from horizontal
A_roof_s = (-0.66 * PF_roof ** 3) + (-0.106 * PF_roof ** 2) + (2.93 * PF_roof)  # Roof is south facing
B_roof_s = (3.63 * PF_roof ** 3) + (-0.374 * PF_roof ** 2) + (-7.4 * PF_roof)
C_roof_s = (-2.71 * PF_roof ** 3) + (-0.991 * PF_roof ** 2) + (4.59 * PF_roof) + 1

if args.debug == 2 or args.debug == 4:
    all_configs_path = f'debug_data/all_configs_{Occupants_Num}_{Location_Input}_{EPC_Space_Heating}_{House_Size}_{TES_Volume_Maximum}_{Temp}_.csv'
    f_all_configs = open(all_configs_path, 'w')

# Heat pump, solar and TES technologies loops
for HP_Option in range(3):  # 0 = Electric Resistant Heater, 1 = ASHP, 2 = GSHP
    for Solar_Option in range(7):  # Flat Plate, Evacuate Tube, 0=None, 1=PV, 2=FP, 3=ET, 4=FP+PV, 5=ET+PV, 6=PVT
        # print(f"H:{HP_Option}, S:{Solar_Option}, T:{time.time() - START_TIME}")
        if Solar_Option == 0:
            Solar_Size_Range = 1
        elif Solar_Option == 4 or Solar_Option == 5:  # One less option when combined
            Solar_Size_Range = Solar_Maximum / 2 - 1
        else:
            Solar_Size_Range = Solar_Maximum / 2
        Optimum_TES_NPC = 1000000
        for Solar_Size in range(int(Solar_Size_Range)):  # 2m2 min up to max
            for TES_Option in range(int(TES_Range)):
                Optimum_Tariff = 1000000
                for Tariff in range(5):  # 5 options: Flat rate, Economy 7, and Bulb Smart, Octopus EV, Octopus Agile
                    Inside_Temp_Current = Temp  # Initial temp
                    Weather_Count = 0
                    Solar_Thermal_Generation_Total = 0
                    Operational_Costs_Peak = 0
                    Operational_Costs_Off_Peak = 0
                    Operation_Emissions = 0

                    TES_Volume_Current = 0.1 + TES_Option * 0.1  # m3
                    TES_Radius = (TES_Volume_Current / (2 * math.pi)) ** (1 / 3)  # For cylinder with height = 2x radius
                    TES_Charge_Full = TES_Volume_Current * 1000 * 4.18 * (Hot_Water_Temp - 40) / 3600  # 40 min temp
                    TES_Charge_Boost = TES_Volume_Current * 1000 * 4.18 * (60 - 40) / 3600  # kWh, 60C HP with PV boost
                    TES_Charge_Max = TES_Volume_Current * 1000 * 4.18 * (95 - 40) / 3600  # kWh, 95C electric and solar
                    TES_Charge_Min = 10 * 4.18 * (Hot_Water_Temp - 10) / 3600  # 10litres hot min amount
                    # CWT coming in from DHW re-fill, accounted for by DHW energy out, DHW min useful temperature 40°C
                    # Space heating return temperature would also be ~40°C with flow at 51°C
                    TES_State_of_Charge = TES_Charge_Full  # kWh, for H2O, starts full to prevent initial demand spike
                    U_Value = 1.30  # 1.30 W/m2K linearised from https://zenodo.org/record/4692649#.YQEbio5KjIV &
                    # https://www.sciencedirect.com/science/article/pii/S0306261916302045

                    if 2 <= Solar_Option:
                        Solar_Thermal_Size = Solar_Size * 2 + 2
                    else:
                        Solar_Thermal_Size = 0
                    if Solar_Option == 1 or Solar_Option == 6:
                        PV_Size = Solar_Size * 2 + 2
                    elif Solar_Option == 4 or Solar_Option == 5:
                        PV_Size = Solar_Maximum - Solar_Thermal_Size
                    else:
                        PV_Size = 0

                    if HP_Option == 0:  # Electrical Resistance Heater
                        cop_worst = 1
                    elif HP_Option == 1:  # ASHP, source A review of domestic heat pumps
                        cop_worst = 6.81 - 0.121 * (Hot_Water_Temp - Coldest_Outside_Temp) + \
                                    0.000630 * (Hot_Water_Temp - Coldest_Outside_Temp) ** 2  # ASHP at coldest temp
                    else:  # GSHP, source A review of domestic heat pumps
                        cop_worst = 8.77 - 0.150 * (Hot_Water_Temp - Ground_Temperature) + \
                                    0.000734 * (Hot_Water_Temp - Ground_Temperature) ** 2  # GSHP ~constant temp at 100m

                    if HP_Option == 0:  # Electrical Resistance Heater
                        HP_Electrical_Power = Boiler_Max_Demand
                    else:  # ASHP or GSHP
                        HP_Electrical_Power = HP_Max_Demand / cop_worst
                    if HP_Electrical_Power * cop_worst < 4.0:  # Mitsubishi have 4kWth ASHP
                        HP_Electrical_Power = 4.0 / cop_worst  # Kensa have 3kWth GSHP
                    if HP_Electrical_Power > 7.0:  # Typical maximum size for domestic power
                        HP_Electrical_Power = 7.0

                    for m in range(12):
                        if m == 3 or m == 5 or m == 8 or m == 10:
                            for d in range(30):
                                function_heater_day_calculation()
                        elif m == 1:
                            for d in range(28):
                                function_heater_day_calculation()
                        else:
                            for d in range(31):
                                function_heater_day_calculation()

                    if False:
                        if HP_Option == 0:  # Electric Heater
                            Current_Options = "Electric Heater " + "%.1f" % (HP_Electrical_Power * cop_worst) + "kW"
                        elif HP_Option == 1:  # ASHP
                            Current_Options = "ASHP power " + "%.1f" % (HP_Electrical_Power * cop_worst) + "kWth"
                        else:  # 2 = GSHP
                            Current_Options = "GSHP power " + "%.1f" % (HP_Electrical_Power * cop_worst) + "kWth"
                        if Solar_Option == 1 or Solar_Option == 4 or Solar_Option == 5:
                            Current_Options += ", PV panels " + "%.0f" % PV_Size + "m2"
                        if Solar_Option == 2 or Solar_Option == 4:
                            Current_Options += ", flat plate solar thermal " + "%.0f" % Solar_Thermal_Size + "m2"
                        elif Solar_Option == 3 or Solar_Option == 5:
                            Current_Options += ", evacuated tube solar thermal " + "%.0f" % Solar_Thermal_Size + "m2"
                        elif Solar_Option == 6:
                            Current_Options += ", PVT panels " + "%.0f" % PV_Size + "m2"
                        Current_Options += ", TES " + ("%.1f" % TES_Volume_Current) + "m3, "
                        if Tariff == 0:
                            Current_Options += "flat tariff OpEx total £" + ("%.2f" % Operational_Costs_Peak)
                        elif Tariff == 1:
                            Current_Options += "economy 7 tariff OpEx peak £" + ("%.2f" % Operational_Costs_Peak) + \
                                               " off peak £" + ("%.2f" % Operational_Costs_Off_Peak) + " total £" + \
                                               ("%.2f" % (Operational_Costs_Peak + Operational_Costs_Off_Peak))
                        elif Tariff == 2:
                            Current_Options += "smart tariff OpEx peak £" + ("%.2f" % Operational_Costs_Peak) + \
                                               " off peak £" + ("%.2f" % Operational_Costs_Off_Peak) + " total £" + \
                                               ("%.2f" % (Operational_Costs_Peak + Operational_Costs_Off_Peak))
                        elif Tariff == 3:
                            Current_Options += "EV tariff OpEx peak £" + ("%.2f" % Operational_Costs_Peak) + \
                                               " off peak £" + ("%.2f" % Operational_Costs_Off_Peak) + " total £" + \
                                               ("%.2f" % (Operational_Costs_Peak + Operational_Costs_Off_Peak))
                        else:
                            Current_Options += "agile tariff OpEx peak £" + ("%.2f" % Operational_Costs_Peak) + \
                                               " off peak £" + ("%.2f" % Operational_Costs_Off_Peak) + " total £" + \
                                               ("%.2f" % (Operational_Costs_Peak + Operational_Costs_Off_Peak))
                        print(Current_Options)

                    # if (Operational_Costs_Peak + Operational_Costs_Off_Peak) < Optimum_Tariff:  # Best for current tech
                    Optimum_Tariff = (Operational_Costs_Peak + Operational_Costs_Off_Peak)
                    if HP_Option == 0:  # Electric Heater
                        CapEx = 100
                        # Small additional cost to a TES, https://zenodo.org/record/4692649#.YQEbio5KjIV
                    else:  # HP
                        if HP_Option == 1:  # ASHP, https://pubs.rsc.org/en/content/articlepdf/2012/ee/c2ee22653g
                            CapEx = ((200 + 4750 / ((HP_Electrical_Power * cop_worst) ** 1.25)) *
                                    (HP_Electrical_Power * cop_worst) + 1500)  # £s
                        else:  # GSHP, https://pubs.rsc.org/en/content/articlepdf/2012/ee/c2ee22653g
                            CapEx = ((200 + 4750 / ((HP_Electrical_Power * cop_worst) ** 1.25)) *
                                    (HP_Electrical_Power * cop_worst) + 800 * (HP_Electrical_Power * cop_worst))

                    if Solar_Option == 1 or Solar_Option == 4 or Solar_Option == 5:  # PV panels installed
                        if (PV_Size * 0.2) < 4.0:  # Less than 4kWp
                            CapEx += PV_Size * 0.2 * 1100  # m2 * 0.2kWp/m2 * £1100/kWp = £
                        else:  # Larger than 4kWp lower £/kWp
                            CapEx += PV_Size * 0.2 * 900  # m2 * 0.2kWp/m2 * £900/kWp = £
                    if Solar_Option >= 2:  # Solar thermal collector
                        if Solar_Option == 2 or Solar_Option == 4:  # Flat plate solar thermal
                            # Technology Library for collector cost https://zenodo.org/record/4692649#.YQEbio5KjIV
                            # Rest from https://www.sciencedirect.com/science/article/pii/S0306261915010958#b0310
                            CapEx += Solar_Thermal_Size * (225 + 270 / (9 * 1.6)) + 490 + 800 + 800
                        elif Solar_Option == 6:  # PVT
                            # https://www.sciencedirect.com/science/article/pii/S0306261915010958#b0310
                            CapEx += (Solar_Thermal_Size / 1.6) * (480 + 270 / 9) + 640 + 490 + 800 + 1440
                        else:  # Evacuated tube solar thermal
                            # Technology Library for collector cost https://zenodo.org/record/4692649#.YQEbio5KjIV
                            # Rest from https://www.sciencedirect.com/science/article/pii/S0306261915010958#b0310
                            CapEx += Solar_Thermal_Size * (280 + 270 / (9 * 1.6)) + 490 + 800 + 800

                    CapEx += 2068.3 * TES_Volume_Current ** 0.553
                    # Formula based on this data https://assets.publishing.service.gov.uk/government/uploads/system/
                    # [rest of address] uploads/attachment_data/file/545249/DELTA_EE_DECC_TES_Final__1_.pdf

                    Net_Present_Cost_Current = CapEx  # £s
                    for Year in range(NPC_Years):  # Optimum for 20 years cost
                        Net_Present_Cost_Current += (Operational_Costs_Peak + Operational_Costs_Off_Peak) / (
                                Discount_Rate ** Year)

                    OpEx = (Operational_Costs_Peak + Operational_Costs_Off_Peak)

                    if args.debug == 2 or args.debug == 4:
                        config_str = f'{HP_Option},{Solar_Option},{PV_Size},{Solar_Thermal_Size},{TES_Volume_Current:.1f},{Tariff},{OpEx:.2f},{CapEx:.2f},{Net_Present_Cost_Current:.2f},{Operation_Emissions:.2f}'
                        f_all_configs.write(config_str + '\n')

                    if Net_Present_Cost_Current < Optimum_TES_NPC:  # Lowest cost TES & tariff for heating tech
                        # For OpEx vs CapEx plots, with optimised TES and tariff
                        Optimum_TES_NPC = Net_Present_Cost_Current
                        Current_TES_and_Tariff_Specifications = [HP_Option, Solar_Option, PV_Size, Solar_Thermal_Size,
                            TES_Volume_Current, Tariff, OpEx, CapEx, Net_Present_Cost_Current, Operation_Emissions]

        Optimum_TES_and_Tariff_Specifications.append(Current_TES_and_Tariff_Specifications)
        # END OF HEATER LOOP

if args.debug == 2 or args.debug == 4:
    f_all_configs.close()

if args.debug == 1 or args.debug == 4:
    optimal_configs_path = f'debug_data/optimal_configs_{Occupants_Num}_{Location_Input}_{EPC_Space_Heating}_{House_Size}_{TES_Volume_Maximum}_{Temp}_.csv'
    f_optimal_configs = open(optimal_configs_path, 'w')

print("Heat Opt, Solar Opt, PV Size, Solar Size, TES Vol, Tariff, OPEX, CAPEX, NPC, Emissions")
for s in Optimum_TES_and_Tariff_Specifications:
    config_str = f'{s[0]},{s[1]},{s[2]},{s[3]},{s[4]:.1f},{s[5]},{s[6]:.2f},{s[7]:.2f},{s[8]:.2f},{s[9]:.2f}'
    print(config_str)
    if args.debug == 1 or args.debug == 4:
        f_optimal_configs.write(config_str + '\n')

if args.debug == 1 or args.debug == 4:
    f_optimal_configs.close()
print("complete_time", time.time() - START_TIME)
