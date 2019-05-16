#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Feb 20 17:31:13 2019

@author: cxyang
"""

import json
import datetime
from requests import get


def request_handler(request):
    #return "Request Dictionary: " + str(request)
    items = get('http://608dev.net/sandbox/sc/lyy/new_test.py').text.split('\n')
    if 'values' in request.keys():
        ip= request['values']['IP']
    else:
        return "Request Dictionary: " + str(request)
    latlong = get('https://ipapi.co/{}/latlong/'.format(ip)).text.split(',')
    response = get('http://api.openweathermap.org/data/2.5/weather?APPID=6fa857fe2f666874f1fb959d3c00c21c&lat={}&lon={}'.format(latlong[0], latlong[1])).json()  
    main = response['weather'][0]['main']
    weather_items = {'Thunderstorm':"umbrella", "Drizzle":"umbrella", "Rain": "umbrella", "Snow":"mittens", "Clouds":"coat", "Clear":"sunscreen"}
    
    if main in weather_items.keys():
        if weather_items[main] in items:
            return "Weather is "+main+", bring "+weather_items[main]+"!"
        else:
            return "Weather is "+main+"!"
    else:
        return "Weather is "+ main
    

#Request Dictionary: {'method': 'POST', 'args': [], 'values': {}, 'is_json': False, 
# 'form': {'{"coord":{"lon":-71.09,"lat":42.36},
    # "weather":[{"id":500,"main":"Rain","description":"light rain","icon":"10n"}],
    # "base":"stations",
    # "main":{"temp":275.19,"pressure":1018,"humidity":64,"temp_min":274.15,"temp_max":276.15},
    # "visibility":16093,"wind":{"speed":2.6,"deg":130},
    # "rain":{"1h":0.48},
    # "clouds":{"all":90},
    # "dt":1551655367,
    # "sys":{"type":1,"id":5255,"message":0.0112,"country":"US","sunrise":1551611717,"sunset":1551652652},
    # "id":6254926,"name":"Massachusetts","cod":200}': ''}}