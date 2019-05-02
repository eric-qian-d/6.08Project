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
        'Authorization': 'Bearer ya29.Glv9BiFrD4VXT8MwcG9ypcfrgm9gfS7ZpFNsAjzsMdn0Q_GiMTwnIcZL_GBF7hVTfVKeDd-dElmsYOXLjNvs-yP4oTr8nK1eWsqJCm3rb16dBAqmP4IZPAxwSWla',
    },
            )
    response.raise_for_status()
    size = len(response.json()["items"])
    result = ""
    for i in range(size-1,size-11,-1):
        result += (response.json()["items"][i]["summary"]) + "\n"
    return result

