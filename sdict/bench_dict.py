db = {} 
for i in xrange(10000001):
	db[str(i)] = i
	if i%10000 ==0 :
		print 'inserting', i

