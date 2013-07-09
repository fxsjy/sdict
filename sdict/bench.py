import tst
db = tst.create_tst_db()
for i in xrange(10000001):
	tst.tst_put(db,str(i),i)
	if i%100000 ==0 :
		print 'inserting', i
for i in xrange(20):
	print tst.tst_get(db,str(i))

