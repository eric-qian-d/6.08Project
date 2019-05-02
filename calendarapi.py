import requests

# If modifying these scopes, delete the file token.pickle.
SCOPES = ['https://www.googleapis.com/auth/calendar.readonly']

def request_handler(request):
    if request['method'] == 'GET':
        return get_events()
        

def get_events():
    
    response = requests.get(
    url='https://www.googleapis.com/calendar/v3/calendars/jenningnchen@gmail.com/events',
    params={
        'q': '',
    },
    headers={
        'Authorization': 'Bearer ya29.Glv9Brfa4YT_XRgxOlMzx1DcKqvf26K3_aihX-U_gqP953e7Vg8sdmgQ_MyKaVR60V8AdrodrGP8ETSX4BExZp6TwAQnY9MuRXv6pUvQhvbAN4wCI9lPydRoLNvA',
    },
            )
    response.raise_for_status()
    size = len(response.json()["items"])
    result = ""
    for i in range(size-1,size-11,-1):
        result += (response.json()["items"][i]["summary"]) + "\n"
    return result

print(get_events());