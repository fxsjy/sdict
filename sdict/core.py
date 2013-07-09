import tst
INT_MAX = (1<<31 - 1)

class Dict(object):

	def __init__(self,py_list=[]):
		self.__is_int_flag__ = 0
		self.__tst__ = tst.create_tst_db()
		py_list = list(py_list)

		if isinstance(py_list,list):
			if len(py_list)>0:
				if not isinstance(py_list[0],tuple):
					raise Exception("invalid list to generate dictionary")

				if isinstance(py_list[0][0],int):
					py_list = [(str(x).zfill(10),y) for x,y in py_list]
					self.__is_int_flag__ = 1
				if not isinstance(py_list[0][0],str):
					raise Exception('invalid key type')

				tst.tst_from_list(self.__tst__, py_list)	
		else:
			raise Exception("need a list, but got a "+str(type(py_list)))

	def __del__(self):
		#print 'over'
		tst.free_tst_db(self.__tst__)

	def put(self,key,value):
		if isinstance(key,str):
			tst.tst_put(self.__tst__,key,value)
		elif isinstance(key,int) and key<(1<<32):
			tst.tst_put(self.__tst__,str(key).zfill(10),value)
			self.__is_int_flag__ = 1
		else:
			raise Exception('invalid key: '+str(key))
	
	def __iter__(self):
		return iter(self.keys())

	def get(self,key):
		if isinstance(key,str):
			return tst.tst_get(self.__tst__,key)
		elif isinstance(key,int) and key<(1<<32):
			return tst.tst_get(self.__tst__,str(key).zfill(10))
		else:
			raise Exception('invalid key: '+str(key))	

	def delete(self,key):
		if isinstance(key,str):
			tst.tst_delete(self.__tst__,key)
		elif isinstance(key,int) and key<(1<<32):
			tst.tst_delete(self.__tst__,str(key).zfill(10))
		else:
			raise Exception('invalid key: '+str(key))

	def prefix(self,key,limit=INT_MAX,is_asc=1):
		result = []
		if isinstance(key,str):
			tst.tst_prefix(self.__tst__,key,result,limit,is_asc)
		else:
			raise Exception('invalid key: '+str(key))	
		return result

	def greater(self,key,limit=INT_MAX):
		result =[]
		int_flag = 0
		if isinstance(key,str):
			tst.tst_greater(self.__tst__,key,result,limit)
		elif isinstance(key,int) and key<(1<<32):
			int_flag = 1
			tst.tst_greater(self.__tst__,str(key).zfill(10),result,limit)
		else:
			raise Exception('invalid key: '+str(key))	
		if int_flag == 1:
			result = [int(x) for x in result]
		return result
	
	def less(self,key,limit=INT_MAX):
		result =[]
		int_flag = 0
		if isinstance(key,str):
			tst.tst_less(self.__tst__,key,result,limit)
		elif isinstance(key,int) and key<(1<<32):
			int_flag =1 
			tst.tst_less(self.__tst__,str(key).zfill(10),result,limit)
		else:
			raise Exception('invalid key: '+str(key))

		if int_flag == 1:
			result = [int(x) for x in result]

		return result
		
	def __getitem__(self,key):
		return self.get(key)

	def __setitem__(self,key,value):
		self.put(key,value)


	def __contains__(self,key):
		return self.get(key)!=None

	def __delitem__(self,key):
		self.delete(key)

	def __len__(self):
		return tst.tst_length(self.__tst__);


	def keys(self):
		items = self.items()
		if self.__is_int_flag__ == 1:
			return [int(x) for x,y in items]

		return [x for (x,y) in items] 

	def values(self):
		items = self.items()
		return [y for (x,y) in items] 
	
	def items(self):
		result =[]
		tst.tst_all(self.__tst__,result)
		return result		
	def __str__(self):
		items = self.items()

		if self.__is_int_flag__ == 1:
			items = [(int(x),y) for x,y in items]	
		return str(dict(items))
	def __repr__(self):
		return self.__str__()

