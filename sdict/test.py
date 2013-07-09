import tst
d = tst.create_tst_db()
print tst.tst_get(d,"xx")
tst.tst_put(d,"abc",range(100))
x = tst.tst_get(d,"abc")
print x
tst.tst_delete(d,"abc")

for i in xrange(10000):
	tst.tst_put(d,str(i),"xxxx"+str(i))

result=[]
key = "16"
limit = 5 

tst.tst_prefix(d,key,result,limit,tst.DESC)
print result

result=[]
limit =5
tst.tst_less(d,key,result,limit)
print result

result=[]
limit =5
tst.tst_greater(d,key,result,limit)
print result












