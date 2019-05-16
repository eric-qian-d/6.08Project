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
		name = str(request['form']['name']).strip('\"')
		description =str(request['form']['description']).strip('\"') if 'description' in request['form'] else ' ' 

		conn = sqlite3.connect(db)
		c = conn.cursor()
		out = ""

		existing_item = c.execute('''SELECT name FROM my_table WHERE item_id = (?);''', (item_id,)).fetchone()
		existing_name = c.execute('''SELECT name FROM my_table WHERE name = (?);''', (name,)).fetchone()

		# if the item with id has already been paired
		if existing_item:
			c.execute('''UPDATE my_table SET name=? WHERE item_id=?;''', (name, item_id))
			c.execute('''UPDATE my_table SET description=? WHERE item_id=?;''', (description, item_id))
			out = "name and description updated for\n" + name

		# if item already exists with this name
		elif existing_name:
			out = "cannot have duplicate name: " + name

		# registering an item
		else:
			if len(name) == 0:
				name = " "
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

		total = []
		for item,item_id,description in table:
			total.append([item, item_id, description])
		sorted_items = sorted(total, key=lambda x:x[0])

		if 'return_id' in request['values']:
			for i in sorted_items:
				out += i[1] + "\n"

		elif 'return_name_id' in request['values']:
			for i in sorted_items:
				out += i[0] + "\n" + i[1]+ "\n"

		else:
			sorted_items = sorted(total, key=lambda x:x[0])
			for i in sorted_items:
				out += i[0] + "\n"

		conn.commit()
		conn.close()
		
		return out