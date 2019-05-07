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
        'Authorization': 'Bearer ya29.Glv9BjWsVR_42fulonrr4shGe5HG94MkHXQBZ42fqxfyojZhKclQzN7yJ6Fs6Tv0m20v7BVP4FHAcC5cbLIg0C-V3RBdsjZWTboKvPQtTyTnC9Yz_Xycg9a4HX8h',
    },
            )
    response.raise_for_status()
    size = len(response.json()["items"])
    result = ""
    for i in range(size-1,size-11,-1):
        result += (response.json()["items"][i]["summary"]) + "\n"
    return result