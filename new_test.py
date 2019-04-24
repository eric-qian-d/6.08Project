import sqlite3

db = '__HOME__/btitems.db'

def request_handler(request):
	conn = sqlite3.connect(db)
	c = conn.cursor()
	c.execute('''CREATE TABLE IF NOT EXISTS my_table (item_id text, name text, description text);''')

	if ('form' in request and 'drop' in request['form']) or ('values' in request and 'drop' in request['values']) :
		
		c.execute('''DROP TABLE my_table;''')
		conn.commit()
		conn.close()
		return "table dropped"

	if request['method'] == 'POST':
		# check all fields are present
		if 'id' not in request['form'] or 'name' not in request['form']:
			return 'need ID, name, and description to pair an item'

		item_id = str(request['form']['id'])
		name = str(request['form']['name'])
		description =str(request['form']['description']) if 'description' in request['form'] else ' ' 

		conn = sqlite3.connect(db)
		c = conn.cursor()
		out = ""

		existing_item = c.execute('''SELECT name FROM my_table WHERE item_id = (?);''', (item_id,)).fetchone()
		existing_name = c.execute('''SELECT name FROM my_table WHERE name = (?);''', (name,)).fetchone()

		# if the item with id has already been paired
		if existing_item:
			out = "item with id " + item_id + " already paired"

		# if item already exists with this name
		elif existing_name:
			out = "cannot have duplicate name: " + name

		# registering an item
		else:
			c.execute('''INSERT into my_table VALUES (?,?,?);''', (item_id, name, description))
			out = name + " successfully paired!"

		conn.commit()
		conn.close()
		return out

	else:
		conn = sqlite3.connect(db)
		c = conn.cursor()
		out = ""
		table = c.execute('''SELECT name,item_id,description FROM my_table;''').fetchall()
		for item,item_id,description in table:
			line = item + " " + item_id + " " + description + "\n"
			out += line
		conn.commit()
		conn.close()
		
		return out
		