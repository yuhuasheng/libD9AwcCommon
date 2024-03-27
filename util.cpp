#include "util.h"
#include <sstream>
#include <iomanip>
using namespace oracle::occi;
//获得指定名称的首选项值
bool  getPreferenceByName(string prefername, vector<string>& preference_vec)
{
	int pref_count = 0;
	int Ifail = ITK_ok;

	//ITK_THROW_ERROR(PREF_set_search_scope(TC_preference_site));
	ITK_THROW_ERROR(Ifail = PREF_ask_value_count(prefername.c_str(), &pref_count));
	cout << "首选项名 == " << prefername << endl;
	if (pref_count > 0)
	{
		int value_count = 0;
		char** pref_values;
		//ITK_THROW_ERROR(PREF_ask_char_values(prefername.c_str(), &value_count, &pref_values)); 
		ITK_THROW_ERROR(PREF_ask_char_values_at_location(prefername.c_str(), TC_preference_site, &value_count, &pref_values));
		if (value_count > 0)
		{
			for (int i = 0; i < value_count; i++)
			{
				cout << "获取到首选项值 == " << pref_values[i] << endl;
				preference_vec.push_back(pref_values[i]);
			}
		}
		else {
			printf("首选项%s配置没有值\n", prefername.c_str());
			//return false;
		}
		if (pref_values)
			MEM_free(pref_values);
	}
	else
	{
		printf("首选项%s没有配置\n", prefername.c_str());
	}

	return Ifail == ITK_ok ? true : false;
}

//获取hashMap首选项
int getHashMapPreference(vector<string> preference_vec, map<string, string>& mp)
{
	char* value = NULL;
	char* result = NULL;
	value = (char*)MEM_alloc(sizeof(char) * 100);
	char* value1 = NULL;
	char* value2 = NULL;
	value1 = (char*)MEM_alloc(sizeof(char) * 100);
	value2 = (char*)MEM_alloc(sizeof(char) * 100);
	int m = 0;
	//string s1 = "";
	//string s2 = "";
	for (int i = 0; i < preference_vec.size(); i++)
	{
		strcpy(value, preference_vec[i].c_str());
		result = strtok(value, "=");
		m = 0;
		while (result != NULL)
		{
			m++;
			if (m == 1) {
				strcpy(value1, result);
			}
			else {
				strcpy(value2, result);
			}
			printf("result == %s\n", result);
			result = strtok(NULL, "=");

		}
		//s1 = value1;
		//s2 = value2;
		mp.insert(pair<string, string>(value1, value2));
	}
	return mp.max_size();

}

// 获取升版前对象版本号集合
vector<string> getReviseBeforeItemRevList(tag_t* rev_list, int count)
{
	char* version = NULL; //版本号
	vector<string> result;
	for (int i = 0; i < count - 1; i++)
	{
		// 获取当前对象版本的版本号
		AOM_ask_value_string(rev_list[i], "item_revision_id", &version);
		result.push_back(version);
	}
	return result;
}

//字符串分割函数 //positions:  x1,y1;x2,y2;x3,y3
vector<string> split(string str, string pattern)
{
	int pos = 0;
	vector<string> result;
	str = str + pattern;
	int len = str.size();

	for (int i = 0; i < len; i++)
	{
		pos = str.find(pattern, i);
		if (pos < len)
		{
			string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}

	}
	return result;
}

string getTemplateId(const vector<string> vec, char* templateName)
{
	if (vec.size() != 0)
	{
		for (int i = 0; i < vec.size(); i++)
		{
			string value = vec[i];
			cout << value << endl;
			vector<string>  array = split(value, "=");
			if (array.size() != 0)
			{
				for (int j = 0; j < array.size(); j++)
				{
					cout << array[j] << endl;
					if (strcmp(array[j].c_str(), templateName) == 0)
					{
						cout << array[j - 1] << endl;
						return  array[j - 1];
					}
				}
			}
		}
	}
	return "";
}

// 利用正则表达式判断字符串的内容
bool regex_matchstr(string search_string, std::tr1::regex reg)
{
	bool flag = std::tr1::regex_match(search_string, reg);
	return flag;
}


void formatDate(char* drawingDate, const char* formdate, date_t date)
{
	char* date_str = NULL;
	DATE_date_to_string(date, formdate, &date_str);
	strcpy(drawingDate, date_str);
	//	tc_strcat(drawingDate, date_str);
	DOFREE(date_str);
	//	TCFREE(date_str);
}


void current_time(date_t* date_tag)
{
	time_t ltime;
	struct tm* today;

	// Set time zone from TZ environment variable. If TZ is not set,
	// the operating system is queried to obtain the default value 
	// for the variable. 
	//
	_tzset();

	// Get UNIX-style time and display as number and string.
	time(&ltime);

	today = localtime(&ltime);
	date_tag->year = today->tm_year + 1900;
	date_tag->month = today->tm_mon;
	date_tag->day = today->tm_mday;
	date_tag->hour = today->tm_hour;
	date_tag->minute = today->tm_min;
	date_tag->second = today->tm_sec;
}

#define MAXSIZE 300
void str_replace(char* str1, char* str2, char* str3) {
	int i, j, k, done, count = 0, gap = 0;
	char temp[MAXSIZE];
	for (i = 0; i < strlen(str1); i += gap) {
		if (str1[i] == str2[0]) {
			done = 0;
			for (j = i, k = 0; k < strlen(str2); j++, k++) {
				if (str1[j] != str2[k]) {
					done = 1;
					gap = k;
					break;
				}
			}
			if (done == 0) { // 已找到待替换字符串并替换
				for (j = i + strlen(str2), k = 0; j < strlen(str1); j++, k++) { // 保存原字符串中剩余的字符
					temp[k] = str1[j];
				}
				temp[k] = '\0'; // 将字符数组变成字符串
				for (j = i, k = 0; k < strlen(str3); j++, k++) { // 字符串替换
					str1[j] = str3[k];
					count++;
				}
				for (k = 0; k < strlen(temp); j++, k++) { // 剩余字符串回接
					str1[j] = temp[k];
				}
				str1[j] = '\0'; // 将字符数组变成字符串
				gap = strlen(str2);
			}
		}
		else {
			gap = 1;
		}
	}
	if (count == 0) {
		printf("Can't find the replaced string!\n");
	}
	return;
}


bool isNumeric(const char* str)
{
	regex reg1("[0-9]*");
	smatch r1;
	string s = str;
	return regex_match(s, r1, reg1);
}

// 判断是否是数字
bool isDigNumeric(const char* str)
{
	regex reg1("[0-9]+\\.[0-9]+");
	smatch r1;
	string s = str;
	return regex_match(s, r1, reg1);
}

char* U2G(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}

char* G2U(const char* gb2312)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}


char* G2B5(char* gb2312)
{

	int len = MultiByteToWideChar(936, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(936, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;

}

int create_dataset(char* ds_type, char* ref_name, char* ds_name, char* fullfilename, char* relation_name, tag_t parent_rev, tag_t* dataset)
{
	int ifail = ITK_ok;
	tag_t ref_object = NULLTAG,
		datasettype = NULLTAG,
		new_ds = NULLTAG,
		tool = NULLTAG,
		folder_tag = NULLTAG;
	AE_reference_type_t reference_type;
	tag_t new_file_tag = NULLTAG;
	IMF_file_t file_descriptor;
	char new_ds_name[WSO_name_size_c + 1] = "";
	char* new_file_name;
	tag_t relation = NULLTAG;
	tag_t spec_relation = NULLTAG;
	char* file_ext = NULL;
	char* filename = NULL;

	if (fullfilename == NULL)
		return ITK_ok;

	file_ext = strrchr(fullfilename, '.') + 1;
	if (file_ext == NULL)
		return ITK_ok;

	filename = strrchr(fullfilename, '\\') + 1;
	if (filename == NULL)
		return ITK_ok;
	new_file_name = USER_new_file_name(new_ds_name, ref_name, file_ext, 0);
	printf("new_file_name == %s\n", new_file_name);
	//new_file_name = USER_new_file_name(new_ds_name, ref_name, ext, 0);
	printf("fullfilename == %s\n", fullfilename);

	if (tc_strcmp(ref_name, "Text") == 0) {
		ifail = IMF_import_file(fullfilename, new_file_name, SS_TEXT, &new_file_tag, &file_descriptor);
	}
	else
	{
		ifail = IMF_import_file(fullfilename, new_file_name, SS_BINARY, &new_file_tag, &file_descriptor);
	}
	if (ifail != ITK_ok)
		return ITK_ok;
	//remove(fullfilename);
	ITK_THROW_ERROR(IMF_set_original_file_name2(new_file_tag, filename));
	ITK_THROW_ERROR(IMF_close_file(file_descriptor));
	ITK_THROW_ERROR(AOM_save_with_extensions(new_file_tag));
	ITK_THROW_ERROR(AOM_unlock(new_file_tag));

	ITK_THROW_ERROR(AE_find_datasettype2(ds_type, &datasettype));
	printf("filename=%s\n", filename);

	ITK_THROW_ERROR(AE_create_dataset_with_id(datasettype, ds_name,
		"", "", "1", &new_ds));
	printf("create dataset sucess\n");
	//CALL( AE_create_dataset ( datasettype, ds_name, "", &new_ds));
	if (FindTargetTool(datasettype, ref_name, &tool) != ITK_ok ||
		tool == NULLTAG)
	{
		ITK_THROW_ERROR(AE_ask_datasettype_def_tool(datasettype, &tool));
	}
	printf("dataset 1\n");
	ITK_THROW_ERROR(AE_set_dataset_tool(new_ds, tool));
	printf("dataset 2\n");
	//ITK_THROW_ERROR(AE_set_dataset_format2(new_ds, BINARY_REF));
	ITK_THROW_ERROR(AE_set_dataset_format2(new_ds, TEXT_REF));
	printf("dataset 3\n");
	ITK_THROW_ERROR(AE_save_myself(new_ds));
	printf("dataset 4\n");
	TC_write_syslog("new_file_tag ==  %d\n", new_file_tag);
	TC_write_syslog("new_ds ==  %d\n", new_ds);
	TC_write_syslog("ref_name ==  %s\n", ref_name);
	ITK_THROW_ERROR(AE_add_dataset_named_ref2(new_ds, ref_name,
		AE_PART_OF, new_file_tag));
	printf("dataset 5\n");
	ITK_THROW_ERROR(AOM_save_with_extensions(new_ds));
	printf("dataset 6\n");
	ITK_THROW_ERROR(AOM_unlock(new_ds));
	*dataset = new_ds;
	//create relation with item revision
	printf("dataset 7\n");
	if (parent_rev != NULL) {
		ITK_THROW_ERROR(GRM_find_relation_type(relation_name, &spec_relation));
		printf("dataset 8\n");
		ITK_THROW_ERROR(GRM_create_relation(parent_rev, new_ds, spec_relation, NULLTAG, &relation));
		printf("dataset 9\n");
		ITK_THROW_ERROR(GRM_save_relation(relation));
		printf("dataset 10\n");
		ITK_THROW_ERROR(AOM_unlock(relation));
		printf("dataset 11\n");
	}

	return ITK_ok;
}

int FindTargetTool(tag_t ds_type, const char* ref_name, tag_t* target_tool)
{
	int ifail = 0, tool_count = 0, j = 0, i = 0, no_refs = 0, * export_flags = NULL;
	tag_t* tool_list = NULL;
	logical found = FALSE;
	char** ref_names = NULL;
	ITK_THROW_ERROR(AE_ask_datasettype_tools(ds_type, &tool_count, &tool_list));


	for (i = 0; i < tool_count; i++)
	{


		ifail = AE_ask_tool_oper_refs(ds_type, tool_list[i], ACTION_open, &no_refs, &ref_names, &export_flags);

		if (ifail == ITK_ok)
		{
			for (j = 0; j < no_refs; j++)
			{
				if (strcmp(ref_names[j], ref_name) == 0 && export_flags[j] == 1)
				{
					*target_tool = tool_list[i];
					found = TRUE;
					break;
				}
			}
		}
		MEM_free(export_flags);
		MEM_free(ref_names);

		if (found)
			break;
	}

	MEM_free(tool_list);

	return ifail;
}

//导入数据集
int import_dataset_file(tag_t dataset, char* ref_name, char* ext, char* fullfilename, char* original_name)
{
	printf("import_dataset_file\n");
	int ifail = ITK_ok;
	tag_t new_file_tag = NULLTAG;
	IMF_file_t file_descriptor = NULL;
	ITK_THROW_ERROR(AOM_refresh(dataset, FALSE));
	char* new_file_name = NULL;
	char new_ds_name[WSO_name_size_c + 1] = "";
	char* filename = NULL;
	new_file_name = USER_new_file_name(new_ds_name, ref_name, ext, 0);
	printf("new_file_name == %s\n", new_file_name);
	filename = strrchr(fullfilename, '\\') + 1;
	if (filename == NULL)
		return ITK_ok;
	fprintf(stdout, "IMF_import_file\n");
	//POM_AM__set_application_bypass(true);
	printf("fullfilename == %s\n", fullfilename);
	if (tc_strcmp(ref_name, "Text") == 0) {
		ITK_THROW_ERROR(IMF_import_file(fullfilename, new_file_name, SS_TEXT, &new_file_tag, &file_descriptor));
	}
	else
	{
		ITK_THROW_ERROR(IMF_import_file(fullfilename, new_file_name, SS_BINARY, &new_file_tag, &file_descriptor));
	}

	fprintf(stdout, "IMF_set_original_file_name\n");
	ITK_THROW_ERROR(IMF_set_original_file_name2(new_file_tag, original_name));
	ITK_THROW_ERROR(IMF_close_file(file_descriptor));
	ITK_THROW_ERROR(AOM_save_without_extensions(new_file_tag));
	ITK_THROW_ERROR(AOM_unlock(new_file_tag));
	ITK_THROW_ERROR(AOM_refresh(new_file_tag, FALSE));

	//添加至命名引用
	POM_AM__set_application_bypass(true);
	ITK_THROW_ERROR(AOM_lock(dataset));
	fprintf(stdout, "AE_remove_dataset_named_ref\n");
	ITK_THROW_ERROR(AE_remove_dataset_named_ref2(dataset, ref_name));
	ITK_THROW_ERROR(AOM_save_without_extensions(dataset));

	fprintf(stdout, "AE_add_dataset_named_ref\n");
	POM_AM__set_application_bypass(true);
	ITK_THROW_ERROR(AE_add_dataset_named_ref2(dataset, ref_name, AE_PART_OF, new_file_tag));
	ITK_THROW_ERROR(AOM_save_without_extensions(dataset));
	ITK_THROW_ERROR(AOM_unlock(dataset));
	//ITK_THROW_ERROR( AOM_refresh( dataset, FALSE ) );
	return ifail;
}


int export_dataset_file(tag_t dataset, char* ref_name, char* ext, char** filename, char** original_name, char* dirPath)
{
	int ifail = ITK_ok;
	tag_t
		ref_object = NULLTAG,
		datasettype = NULLTAG,
		new_ds = NULLTAG,
		tool = NULLTAG,
		folder_tag = NULLTAG,
		spec_dataset_rev = NULLTAG;
	AE_reference_type_t reference_type;
	tag_t new_file_tag = NULLTAG;
	IMF_file_t file_descriptor;
	char new_ds_name[WSO_name_size_c + 1] = "";
	char* new_file_name;

	*filename = (char*)MEM_alloc(sizeof(char) * BUFSIZE);
	*original_name = (char*)MEM_alloc(sizeof(char) * BUFSIZE);
	strcpy(*filename, "");
	ITK_THROW_ERROR(AE_ask_dataset_latest_rev(dataset, &spec_dataset_rev));
	ITK_THROW_ERROR(AE_ask_dataset_named_ref2(dataset, ref_name, &reference_type, &ref_object));
	if (ref_object == NULLTAG)
	{
		fprintf(stdout, "ref_object is NULLTAG\n");
		return 1;
	}

	if (reference_type == AE_PART_OF)
	{
		//char pathname[SS_MAXPATHLEN] = "";
		char* pathname = NULL;
		//ITK_THROW_ERROR(IMF_ask_file_pathname(ref_object, SS_WNT_MACHINE, pathname));
		ITK_THROW_ERROR(IMF_ask_file_pathname2(ref_object, SS_WNT_MACHINE, &pathname));
		//char origin_file_name[IMF_filename_size_c + 1] = "";
		char* origin_file_name = NULL;
		//ITK_THROW_ERROR(IMF_ask_original_file_name(ref_object, origin_file_name));
		ITK_THROW_ERROR(IMF_ask_original_file_name2(ref_object, &origin_file_name));
		strcpy(*original_name, origin_file_name);
		char new_ds_name[WSO_name_size_c + 1] = "";
		char* new_file_name = USER_new_file_name(new_ds_name, ref_name, ext, 0);
		//char *temp_dir = getenv(EvnPDF.c_str()); //"TEMP"
		char temp_file[SS_MAXPATHLEN] = "";
		/*strcpy(temp_file, temp_dir);
		strcat(temp_file, "\\");
		strcat(temp_file, new_file_name);*/
		strcpy(temp_file, dirPath);
		strcat(temp_file, "\\");
		strcat(temp_file, new_file_name);
		//strcpy(temp_file,dirPath);
		fprintf(stdout, "temp_file=%s\n", temp_file);
		ITK_THROW_ERROR(IMF_export_file(ref_object, temp_file));
		strcpy(*filename, temp_file);
	}

	return ITK_ok;
}

int execmd(char* cmd, char* result) {
	char buffer[128];                         //定义缓冲区                        
	FILE* pipe = _popen(cmd, "r");            //打开管道，并执行命令
	if (!pipe) return 0;                      //返回0表示运行失败
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe)) {             //将管道输出到result中
			strcat(result, buffer);
		}
	}
	_pclose(pipe);                            //关闭管道
	return 1;                                 //返回1表示运行成功
}

/************************************************************************/
/* Function: 按条件查询对象
/* argument：qry_name job_name
/* argument：attr_names 条件类型
/* argument：values 条件值
/* argument：attr_cnt 条件个数
/* argument：results 查找获取的对象
/* argument: results_cnt 获取对象的个数
/************************************************************************/
int query(char* qry_name, char** attr_names, char** values, int attr_cnt, tag_t** results, int* results_cnt)
{
	int rcode = ITK_ok;
	int i = 0;
	int j = 0;
	tag_t qry = NULLTAG;
	// QRY_describe_query
	int num_clauses = 0;
	char** attr_names_qry = NULL;
	char** entry_names = NULL;
	char** logical_ops = NULL;
	char** math_ops = NULL;
	char** values_qry = NULL;
	tag_t* lov_tags = NULL;
	int* attr_types = NULL;


	char** qry_entry = NULL;
	char** qry_value = NULL;

	//char** keys = {0};
	char** keys = NULL;
	int keys_num = 1;
	int flag_ok = 0;
	/*int *sort = (int *) MEM_alloc( sizeof( int ) * 1 );
	*sort=1;
	keys = (char **) MEM_alloc( sizeof( char *) * 1 );
	keys[0] = (char *) MEM_alloc( sizeof( char ) * 256 );
	strcpy(keys[0],"item_id");*/
	//检查入参值
	//fprintf(stdout,"-------------enter yf_query function!------------\n");
	fprintf(stdout, "      qry_name = %s\n", qry_name);
	/*
	for(int k = 0; k < attr_cnt; k++)
	{
	fprintf(stdout,"      attr_names = %s\n",attr_names[k]);
	fprintf(stdout,"      values = %s\n",values[k]);
	}
	*/
	TC_write_syslog(" qry_name = %s\n", qry_name);
	for (i = 0; i < attr_cnt; i++)
	{
		TC_write_syslog("	attr_names[%d] = %s			values[%d] = %s\n", i, attr_names[i], i, values[i]);
	}
	//fprintf(stdout,"-------------find !------------\n");
	ITK_THROW_ERROR(QRY_find2(qry_name, &qry));
	fprintf(stdout, "      QRY_find qry = %d\n", qry);
	if (qry != NULL)
	{
		qry_entry = (char**)MEM_alloc(sizeof(char*) * attr_cnt);
		qry_value = (char**)MEM_alloc(sizeof(char*) * attr_cnt);

		for (i = 0; i < attr_cnt; i++)
		{
			qry_entry[i] = (char*)MEM_alloc(sizeof(char) * 256);
			memset(qry_entry[i], 0, sizeof(char) * 256);
			tc_strcpy(qry_entry[i], attr_names[i]);

			qry_value[i] = (char*)MEM_alloc(sizeof(char) * 256);
			memset(qry_value[i], 0, sizeof(char) * 256);
			tc_strcpy(qry_value[i], values[i]);
		}


		//ITK_THROW_ERROR( QRY_execute( qry , attr_cnt , qry_entry , values , results_cnt , results ));
		ITK_THROW_ERROR(QRY_execute(qry, attr_cnt, qry_entry, qry_value, results_cnt, results));
		//ITK_THROW_ERROR(QRY_execute_with_sort(qry, attr_cnt, qry_entry, qry_value,keys_num,keys,sort,results_cnt, results));
		//ITK_THROW_ERROR( QRY_execute( qry , attr_cnt , attr_names , values , results_cnt , results ));
		//if(flag_ok!=0)
		//ITK_THROW_ERROR( QRY_execute( qry , flag_ok , qry_entry , qry_value , results_cnt , results ));
		TC_write_syslog("	====>	num_found = %d\n", *results_cnt);


		DOFREE(qry_entry);
		DOFREE(qry_value);

	}
	else
	{
		TC_write_syslog("--->		can't find qry %s\n", qry_name);
	}
	TC_write_syslog("-------------------- qtmc_qry_service end ----------------------\n");
	fprintf(stdout, "-------------------- qtmc_qry_service end ----------------------\n");
	return ITK_ok;
}

// 判断对象的类型
logical checkItemType(vector<string>  tagTypeVec, tag_t rev_tag)
{
	tag_t type_tag = NULLTAG;
	tag_t item_type = NULLTAG;
	logical isItemType = false;
	ITK_THROW_ERROR(TCTYPE_ask_object_type(rev_tag, &type_tag));
	for (int i = 0; i < tagTypeVec.size(); i++)
	{
		ITK_THROW_ERROR(TCTYPE_find_type(tagTypeVec[i].c_str(), NULL, &item_type));
		ITK_THROW_ERROR(TCTYPE_is_type_of(type_tag, item_type, &isItemType));
		if (isItemType)
		{
			return true;
		}
	}
	return  false;
}

char* getProjectInfo(tag_t item_tag)
{
	int projectNum = 0;
	tag_t* projects = NULL;
	char* projectId = NULL;
	ITK_THROW_ERROR(AOM_ask_value_tags(item_tag, "project_list", &projectNum, &projects));
	if (projectNum != 0) {
		ITK_THROW_ERROR(AOM_ask_value_string(projects[0], "project_id", &projectId));
		//printf("projectId：%s", projectId);
		cout << "projectId：" << projectId << endl;
	}

	return projectId;
}

bool getProjectInfo(tag_t item_tag, vector<string>& preference_vec)
{
	int
		Ifail = ITK_ok,
		projectNum = 0;

	tag_t* projects = NULL;

	char* projectId = NULL;
	ITK_THROW_ERROR(AOM_ask_value_tags(item_tag, "project_list", &projectNum, &projects));
	printf("projectNum size is " + projectNum);
	if (projectNum != 0) {
		for (int i = 0; i < projectNum; i++)
		{
			ITK_THROW_ERROR(AOM_ask_value_string(projects[i], "project_id", &projectId));
			cout << "获取到首选项值 == " << projectId << endl;
			preference_vec.push_back(projectId);
		}

		if (projectId)
			DOFREE(projectId);
	}

CLEANUP:
	DOFREE(projects);

	return Ifail == ITK_ok ? true : false;
}


int getProjectInfo(tag_t item_tag, vector<tag_t>& vec) {
	int
		rcode = ITK_ok,
		projectNum = 0;
	tag_t* projects = NULL;
	char* projectId = NULL;

	ITK_THROW_ERROR(AOM_ask_value_tags(item_tag, "project_list", &projectNum, &projects));
	printf("projectNum size is " + projectNum);
	for (size_t i = 0; i < projectNum; i++) {
		vec.push_back(projects[i]);
	}

CLEANUP:
	DOFREE(projects);
	return rcode;
}


int GTCTYPE_is_type_of(tag_t object_tag, const char* typeName, logical* isType)
{
	int		ifail = ITK_ok;
	char* err_string;
	int		err_line = -1;
	tag_t   typeTag = NULLTAG;

	*isType = false;
	if (object_tag == NULLTAG)
		goto CLEANUP;
	ITK_THROW_ERROR(TCTYPE_find_type(typeName, NULL, &typeTag));
	if (typeTag != NULLTAG) {
		ITK_THROW_ERROR(ifail = AOM_is_type_of(object_tag, typeTag, isType));
	}

CLEANUP:
	return ifail;
}




int checkTagAttr(vector<string>  tagAttrVec, tag_t  rev_tag, char** error_msg) {
	tag_t v_tag = NULL;
	int  ifail = ITK_ok, proCnt = 0, isFind = 0, refer_num = 0;
	char	tempStr[256];
	char* object_string = NULL, * object_id = NULL;
	char* disp_prop_name = NULL;
	char** pros = NULL;
	tag_t* refer_list = NULL;
	ITK_THROW_ERROR(AOM_ask_value_string(rev_tag, "object_name", &object_string));
	ITK_THROW_ERROR(AOM_ask_value_string(rev_tag, "item_id", &object_id));

	AOM_ask_prop_names(rev_tag, &proCnt, &pros);
	if (proCnt == NULL || proCnt <= 0) {
		goto  CLEANUP;
	}

	for (int i = 0; i < tagAttrVec.size(); i++)
	{
		isFind = 0;
		for (int j = 0; j < proCnt; j++) {
			if (tc_strcmp(pros[j], tagAttrVec[i].c_str()) == 0) {
				isFind = 1;
				printf("找到了对象属性%s\n", pros[j]);
				TC_write_syslog("找到了对象属性%s\n", pros[j]);
				break;
			}
		}

		if (isFind == 0) {
			continue;
		}
		AOM_load(rev_tag);
		ITK_THROW_ERROR(AOM_ask_value_tags(rev_tag, tagAttrVec[i].c_str(), &refer_num, &refer_list));
		if (refer_num == 0) {
			ITK_THROW_ERROR(AOM_UIF_ask_name(rev_tag, tagAttrVec[i].c_str(), &disp_prop_name));
			memset(tempStr, '\0', 256);
			sprintf(tempStr, G2B5("对象:%s/%s,属性:%s,值为空,请填写!\n"), object_id, object_string, disp_prop_name);
			printf(tempStr);
			TC_write_syslog(G2B5("对象:%s/%s,属性:%s,值为空,请填写!\n"), object_id, object_string, disp_prop_name);
			tc_strcat(*error_msg, tempStr);
			printf(*error_msg);
		}
	}

CLEANUP:
	DOFREE(pros);
	DOFREE(disp_prop_name);

	return ITK_ok;
}


int checkStrAttr(vector<string>  tagAttrVec, tag_t  rev_tag, char** error_msg) {
	tag_t v_tag = NULL;
	int  ifail = ITK_ok, proCnt = 0, isFind = 0, refer_num = 0;
	char	tempStr[256];
	char* object_string = NULL, * object_id = NULL;
	char* disp_prop_name = NULL, * v;
	char** pros = NULL;
	tag_t* refer_list = NULL;
	ITK_THROW_ERROR(AOM_ask_value_string(rev_tag, "object_name", &object_string));
	ITK_THROW_ERROR(AOM_ask_value_string(rev_tag, "item_id", &object_id));

	AOM_ask_prop_names(rev_tag, &proCnt, &pros);
	if (proCnt == NULL || proCnt <= 0) {
		goto  CLEANUP;
	}

	for (int i = 0; i < tagAttrVec.size(); i++)
	{
		isFind = 0;
		for (int j = 0; j < proCnt; j++) {
			if (tc_strcmp(pros[j], tagAttrVec[i].c_str()) == 0) {
				isFind = 1;
				printf("找到了对象属性%s\n", pros[j]);
				TC_write_syslog("找到了对象属性%s\n", pros[j]);
				break;
			}
		}

		if (isFind == 0) {
			continue;
		}
		AOM_load(rev_tag);
		ITK_THROW_ERROR(AOM_ask_value_string(rev_tag, tagAttrVec[i].c_str(), &v));
		if (tc_strcmp(v, "") == 0) {
			ITK_THROW_ERROR(AOM_UIF_ask_name(rev_tag, tagAttrVec[i].c_str(), &disp_prop_name));
			memset(tempStr, '\0', 256);
			sprintf(tempStr, G2B5("对象:%s/%s,属性:%s,值为空,请填写!\n"), object_id, object_string, disp_prop_name);
			TC_write_syslog(G2B5("对象:%s/%s,属性:%s,值为空,请填写!\n"), object_id, object_string, disp_prop_name);
			tc_strcat(*error_msg, tempStr);
			printf(*error_msg);
		}
	}

CLEANUP:
	DOFREE(pros);
	DOFREE(disp_prop_name);

	return ITK_ok;
}

string getMaterialGroup(char* groupName, vector<string> preference_vec) {

	for (int i = 0; i < preference_vec.size(); i++)
	{
		vector<string> kvVec = split(preference_vec[i], "=");
		string k = kvVec[0];
		string v = kvVec[1];

		if (strstr(groupName, k.c_str())) {
			return v.c_str();
		}
	}

	return "";

}

/**
 判断是否含有BOM视图
*/
logical checkStructure(tag_t itemRev)
{
	int structCount = 0;
	tag_t* struct_tag_list = NULLTAG;
	ITK_THROW_ERROR(AOM_ask_value_tags(itemRev, "structure_revisions", &structCount, &struct_tag_list));
	if (structCount > 0)
	{
		return true;
	}
	return false;
}

/*
判断是否含有子BOMLine
*/
logical hasChildBOMLine(tag_t tItemRev)
{
	tag_t
		window = NULL_TAG,
		top_bomline = NULL_TAG;
	int
		child_count = 0,
		bvrs_count = 0;
	tag_t
		* child_lines = NULL,
		* bvrs_tag = NULL;


	ITK_THROW_ERROR(ITEM_rev_list_all_bom_view_revs(tItemRev, &bvrs_count, &bvrs_tag));
	if (bvrs_count == 0)
	{
		return false;
	}

	ITK_THROW_ERROR(BOM_create_window(&window)); // 创建BOMWindow
	ITK_THROW_ERROR(BOM_set_window_top_line_bvr(window, bvrs_tag[0], &top_bomline));
	ITK_THROW_ERROR(BOM_line_ask_child_lines(top_bomline, &child_count, &child_lines));
	ITK_THROW_ERROR(BOM_close_window(window)); // 关闭BOMWindow
	if (child_count > 0)
	{
		return true;
	}
	return false;
}


int startWith(char* str, char* p) {
	int len = strlen(p);
	if (len <= 0)
		return 0;
	if (strncmp(str, p, len) == 0)
		return 1;
	return 0;
}


int endWith(char* str, char* p) {
	int len1, len2;
	len1 = strlen(str);
	len2 = strlen(p);

	if (len2 <= 0) {
		return 0;
	}

	if (strncmp(str + len1 - len2, p, len2) == 0) {
		return 1;
	}

	return 0;
}

int containStr(char* str, char* p) {
	int len1, len2;
	len1 = strlen(str);
	len2 = strlen(p);

	if (len1 <= 0) {
		return 0;
	}

	if (len2 <= 0) {
		return 0;
	}

	if (strstr(str, p) == NULL) {
		return 0;
	} else {
		return 1;
	}

}

// 将strRes中的from替换为to，替换成功返回1，否则返回0。
int strReplace(char strRes[], char from[], char to[]) {
	int i, flag = 0;
	char* p, * q, * ts;
	for (i = 0; strRes[i]; ++i) {
		if (strRes[i] == from[0]) {
			p = strRes + i;
			q = from;
			while (*q && (*p++ == *q++));
			if (*q == '\0') {
				ts = (char*)malloc(strlen(strRes) + 1);
				strcpy(ts, p);
				strRes[i] = '\0';
				strcat(strRes, to);
				strcat(strRes, ts);
				free(ts);
				flag = 1;
			}
		}
	}
	return flag;
}

bool startWithStr(const std::string& str, const std::string prefix) {
	return (str.rfind(prefix, 0) == 0);
}
bool endWithStr(const std::string& str, const std::string suffix) {
	if (suffix.length() > str.length()) { return false; }
	return (str.rfind(suffix) == (str.length() - suffix.length()));
}

bool isContainsStr(const std::string& str, const std::string contains_str) {
	string::size_type idx = str.find(contains_str);
	if (idx != string::npos) {
		return true;
	}
	return false;
}


void string_replace(string& s1, const string& s2, const string& s3)
{
	string::size_type pos = 0;
	string::size_type a = s2.size();
	string::size_type b = s3.size();
	while ((pos = s1.find(s2, pos)) != string::npos)
	{
		s1.replace(pos, a, s3);
		pos += b;
	}
}


//设置字典
static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

//base64 编码
std::string base64_encode(char const* bytes_to_encode, int in_len) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';

	}

	return ret;

}

//base64 解码
std::string base64_decode(std::string& encoded_string) {
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}

/*
判断属性是否存在
*/
logical checkProName(tag_t primaryTag, char* proName) {
	int prop_count = 0;
	char** prop_names = NULL;
	logical has_prop = false;
	int j = 0;

	AOM_ask_prop_names(primaryTag, &prop_count, &prop_names);

	for (j = 0; j < prop_count; j++) {
		if (tc_strcmp(prop_names[j], proName) == 0) {
			has_prop = true;
			break;
		}
	}

	if (prop_names)
		DOFREE(prop_names);
	return has_prop;
}


/*
* 创建文件夹
*/
void createFolder(string dir)
{
	if (_access(dir.c_str(), 0) == -1)
	{
		_mkdir(dir.c_str());
	}
	system("pause");
}

/*
* 创建临时目录文件夹
*/
std::string getTempFolder(std::string foldName) {
	char* temp = getenv("TEMP"); // 获取用户下的temp目录
	string dir = (string)temp + "\\" + foldName;

	createFolder(dir); // 创建文件夹

	return dir;
}

/*
* 创建文件
*/
bool createFile(std::string fileName) {
	FILE* fp;
	if ((fp = fopen(fileName.c_str(), "wb")) == NULL)
	{
		printf("%s can't be create\n", fileName.c_str());
		return false;
	}
	else
	{
		fclose(fp);

	}
	system("pause");
	return true;
}

/*
* 文件追加内容
*/
void appendContent(const char* fileName, vector<string> content) {
	FILE* fp;
	if (_access(fileName, 0) == 0) { // 判断文件是否存在
		if ((fp = fopen(fileName, "a+")) != NULL) { // 选用追加模式
			for (size_t i = 0; i < content.size(); i++)
			{
				fprintf(fp, "%s\n", content[i].c_str()); //将内容以字符串形式写入文件
			}

			fclose(fp); //关闭文件
		}
	}
}


/*
* 删除空文件夹
*/
void deleteFolder(string dir)
{
	if (_access(dir.c_str(), 0) == 0)
	{
		_rmdir(dir.c_str());
	}
}


//判断是否是".."目录和"."目录
bool is_special_dir(const char* path)
{
	return strcmp(path, "..") == 0 || strcmp(path, ".") == 0;
}

//判断文件属性是目录还是文件
bool is_dir(int attrib)
{
	return attrib == 16 || attrib == 18 || attrib == 20;
}

//显示删除失败原因
void show_error(const char* file_name = NULL)
{
	errno_t err;
	_get_errno(&err);
	switch (err)
	{
	case ENOTEMPTY:
		printf("Given path is not a directory, the directory is not empty, or the directory is either the current working directory or the root directory.\n");
		break;
	case ENOENT:
		printf("Path is invalid.\n");
		break;
	case EACCES:
		printf("%s had been opend by some application, can't delete.\n", file_name);
		break;
	}
}

void get_file_path(const char* path, const char* file_name, char* file_path)
{
	strcpy_s(file_path, sizeof(char) * _MAX_PATH, path);
	file_path[strlen(file_path) - 1] = '\0';
	strcat_s(file_path, sizeof(char) * _MAX_PATH, file_name);
	strcat_s(file_path, sizeof(char) * _MAX_PATH, "\\*");
}

//递归搜索目录中文件并删除
void delete_file(const char* path)
{
	_finddata_t dir_info;
	_finddata_t file_info;
	intptr_t f_handle;
	char tmp_path[_MAX_PATH];
	if ((f_handle = _findfirst(path, &dir_info)) != 0)
	{
		while (_findnext(f_handle, &file_info) == 0)
		{
			if (is_special_dir(file_info.name))
			{
				continue;
			}
			if (is_dir(file_info.attrib))//如果是目录，生成完整的路径
			{
				get_file_path(path, file_info.name, tmp_path);
				delete_file(tmp_path);//开始递归删除目录中的内容
				tmp_path[strlen(tmp_path) - 2] = '\0';
				if (file_info.attrib == 20)
					printf("This is system file, can't delete!\n");
				else
				{
					//删除空目录，必须在递归返回前调用_findclose,否则无法删除目录
					if (_rmdir(tmp_path) == -1)
					{
						show_error();//目录非空则会显示出错原因
					}
				}
			}
			else
			{
				strcpy_s(tmp_path, path);
				tmp_path[strlen(tmp_path) - 1] = '\0';
				strcat_s(tmp_path, file_info.name);//生成完整的文件路径

				if (remove(tmp_path) == -1)
				{
					show_error(file_info.name);
				}

			}
		}
		_findclose(f_handle);//关闭打开的文件句柄，并释放关联资源，否则无法删除空目录
	}
	else
	{
		show_error();//若路径不存在，显示错误信息
	}

	return;
}

/*
* 递归删除目录下所有文件(window)
*/
void removeDir(const char* dirPath)
{
	struct _finddata_t fb;   // 查找相同属性文件的存储结构体
	char  path[250];
	long long  handle; // 注意此处需要long*2
	int   noFile;            // 对系统隐藏文件的处理标记
	noFile = 0;
	handle = 0;
	// 制作路径
	strcpy(path, dirPath);
	strcat(path, "/*");
	handle = _findfirst(path, &fb);
	// 找到第一个匹配的文件
	if (handle != 0)
	{
		// 当可以继续找到匹配的文件，继续执行
		while (0 == _findnext(handle, &fb))
		{
			// windows下，常有个系统文件，名为“..”,对它不做处理
			noFile = strcmp(fb.name, "..");
			if (0 != noFile)
			{
				// 制作完整路径
				memset(path, 0, sizeof(path));
				strcpy(path, dirPath);
				strcat(path, "/");
				strcat(path, fb.name);
				// 属性值为16，则说明是文件夹，迭代
				if (fb.attrib == 16)
				{
					removeDir(path);
				}
				// 非文件夹的文件，直接删除。对文件属性值的情况没做详细调查，可能还有其他情况。
				else
				{
					remove(path);
				}
			}
		}
		// 关闭文件夹，只有关闭了才能删除。找这个函数找了很久，标准c中用的是closedir
		// 经验介绍：一般产生Handle的函数执行后，都要进行关闭的动作。
		_findclose(handle);
	}
}

void getFiles(string path, vector<string>& files)
{
	//文件句柄
	long   long  hFile = 0;
	//文件信息
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之
			//如果不是,加入列表
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

// 文件重命名
boolean rename(string oldName, string newName)
{
	fstream f;
	f.open(oldName.c_str());

	if (f.fail())
	{
		std::cout << "File Open Failed!" << std::endl;
		f.close();
		return false;
	}

	else
	{
		f.close();
		if (-1 == rename(oldName.c_str(), newName.c_str()))
		{
			std::cout << "file rename failed!" << std::endl;
		}

		//f.close();
	}
	system("Pause");
	return true;
}

void getConnection(string prefername, Environment*& env, Connection*& conn) {
	char* ip = new char[32];
	char* userName = new char[32];
	char* password = new char[32];
	char* port = new char[32];
	char* sid = new char[32];
	char* connectStr = new char[128];
	strcpy(connectStr, "");

	vector<string>* dbInfoVec = new vector<string>();
	map<string, string>* dbInfoMap = new map<string, string>();
	map<string, string>::reverse_iterator* dbInfoIter = new map<string, string>::reverse_iterator();

	getPreferenceByName(prefername, *dbInfoVec);
	getHashMapPreference(*dbInfoVec, *dbInfoMap);

	for ((*dbInfoIter) = (*dbInfoMap).rbegin(); (*dbInfoIter) != (*dbInfoMap).rend(); (*dbInfoIter)++) {
		string tmpKey = (*dbInfoIter)->first;
		string tmpValue = (*dbInfoIter)->second;
		if (strcmp(tmpKey.c_str(), "IP") == 0) {
			printf("ip decoded== %s\n", base64_decode(tmpValue));
			strcpy(ip, base64_decode(tmpValue).c_str());
		}
		else if (strcmp(tmpKey.c_str(), "UserName") == 0) {
			printf("userName decoded== %s\n", base64_decode(tmpValue));
			strcpy(userName, base64_decode(tmpValue).c_str());
		}

		else if (strcmp(tmpKey.c_str(), "username") == 0) {
			printf("userName decoded== %s\n", base64_decode(tmpValue));
			strcpy(userName, base64_decode(tmpValue).c_str());
		}

		else if (strcmp(tmpKey.c_str(), "Password") == 0) {
			printf("password decoded== %s\n", base64_decode(tmpValue));
			strcpy(password, base64_decode(tmpValue).c_str());
		}

		else if (strcmp(tmpKey.c_str(), "password") == 0) {
			printf("password decoded== %s\n", base64_decode(tmpValue));
			strcpy(password, base64_decode(tmpValue).c_str());
		}

		else if (strcmp(tmpKey.c_str(), "Port") == 0) {
			printf("port decoded== %s\n", base64_decode(tmpValue));
			strcpy(port, base64_decode(tmpValue).c_str());
		}
		else if (strcmp(tmpKey.c_str(), "SID") == 0) {
			printf("sid decoded== %s\n", base64_decode(tmpValue));
			strcpy(sid, base64_decode(tmpValue).c_str());
		}
	}

	printf("ip == %s\n", ip);
	TC_write_syslog("ip == %s\n", ip);
	printf("userName == %s\n", userName);
	TC_write_syslog("userName == %s\n", userName);
	printf("password == %s\n", password);
	TC_write_syslog("password == %s\n", password);
	printf("port == %s\n", port);
	TC_write_syslog("port == %s\n", port);
	printf("sid == %s\n", sid);
	TC_write_syslog("sid == %s\n", sid);

	tc_strcat(connectStr, ip);
	tc_strcat(connectStr, ":");
	tc_strcat(connectStr, port);
	tc_strcat(connectStr, "/");
	tc_strcat(connectStr, sid);
	printf("connectStr == %s\n", connectStr);
	TC_write_syslog("connectStr == %s\n", connectStr);

	env = Environment::createEnvironment();
	if (NULL == env) {
		printf("createEnvironment error... \n");
		TC_write_syslog("createEnvironment error... \n");
	}

	printf("createEnvironment success... \n");
	TC_write_syslog("createEnvironment success... \n");

	conn = env->createConnection(userName, password, connectStr);
	if (NULL == conn) {
		printf("createConnection error... \n");
		TC_write_syslog("createConnection error... \n");
	}

	printf("createConnection success... \n");
	TC_write_syslog("createConnection success... \n");

CLEANUP:
	if (dbInfoVec != NULL)
	{
		(*dbInfoVec).clear();
		dbInfoVec = NULL;
	}

	if (dbInfoMap != NULL)
	{
		(*dbInfoMap).clear();
		dbInfoMap = NULL;

	}
	dbInfoIter = NULL;
}

/*
* 校验属性是否存在
*/
int validateProp(tag_t object_tag, const char* propertyName, logical* verdict) {
	int ifail = ITK_ok;
	char* type_name;
	tag_t tagType = NULL;
	ITK_THROW_ERROR(WSOM_ask_object_type2(object_tag, &type_name));
	TC_write_syslog("type_name == %s\n", type_name);
	ITK_THROW_ERROR(TCTYPE_ask_type(type_name, &tagType));
	TCTYPE_property_exists(tagType, propertyName, verdict);
	return ifail;
}


bool isBom(tag_t object_tag) {
	logical
		flag = false;

	tag_t
		* children = NULLTAG;
	int
		count = 0;

	AOM_ask_value_tags(object_tag, "ps_children", &count, &children);
	if (count > 0) {
		flag = true;
	}

CLEANUP:
	DOFREE(children);
	return flag;
}

size_t ReceiveData(void* contents, size_t size, size_t nmemb, void* stream) {
	std::string* str = (std::string*)stream;
	(*str).append((char*)contents, size * nmemb);
	return size * nmemb;
}

CURLcode httpPostFormData(const std::string& url, const std::string& data, std::string& response, int timeout) {
	CURLcode res;
	CURL* curl = curl_easy_init();
	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, "Content-Type:application/x-www-form-urlencoded;charset=UTF-8");

	if (curl == NULL)
	{
		return CURLE_FAILED_INIT;
	}

	TC_write_syslog("http curl :: %d\n", curl);

	// 设置请求地址
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	// 设置请求头信息
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	// 不显示接收头信息
	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	// 设置连接超时时间
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);
	// 设置请求超时时间
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
	// 设置请求体
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
	// 设置接收函数
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ReceiveData);
	// 设置接收内容
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);

	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

CURLcode httpPost(const std::string& url, const std::string& data, std::string& response, int timeout) {
	CURLcode res;
	CURL* curl = curl_easy_init();
	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");

	if (curl == NULL)
	{
		return CURLE_FAILED_INIT;
	}

	TC_write_syslog("http curl :: %d\n", curl);

	// 设置请求地址
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	// 设置请求头信息
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	// 不显示接收头信息
	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	// 设置连接超时时间
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);
	// 设置请求超时时间
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
	// 设置请求体
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
	// 设置接收函数
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ReceiveData);
	// 设置接收内容
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);

	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}


CURLcode httpGet(const std::string& url, std::string& response, int timeout) {
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (curl == NULL) {
		return CURLE_FAILED_INIT;
	}

	TC_write_syslog("http curl :: %d\n", curl);
	// 设置请求地址
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	// 设置连接超时时间
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);
	//  设置请求超时时间
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

	// 设置接收函数
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ReceiveData);
	// 设置接收内容
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);

	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	return res;
}

bool isRelease(tag_t object_tag) {
	bool flag = false;
	int status_num = 0;
	tag_t* status_list = NULL;

	if (object_tag == NULL) {
		goto CLEANUP;
	}

	ITK_THROW_ERROR(AOM_ask_value_tags(object_tag, release_status_list, &status_num, &status_list));
	if (status_num > 0) {
		flag = true;
	}

CLEANUP:
	DOFREE(status_list);
	return flag;

}


/*
* 获取状态值
*/
string getReleaseName(tag_t object_tag) {
	int
		status_num = 0;

	tag_t* status_list = NULL;

	string
		statusValue = "";
	char
		* value = NULL;

	if (object_tag == NULL) {
		goto CLEANUP;
	}

	ITK_THROW_ERROR(AOM_ask_value_tags(object_tag, release_status_list, &status_num, &status_list));

	for (int i = 0; i < status_num; i++) {
		tag_t tag = status_list[i];
		ITK_THROW_ERROR(AOM_ask_value_string(tag, object_name, &value));

		if (NULL != value) {
			statusValue = getTcCode(value);
			break;
		}
	}

CLEANUP:
	DOFREE(status_list);
	DOFREE(value);
	return statusValue;
}


/*
* 判断是否含有签核记录
*/
bool checkSignRecord(tag_t object_tag) {
	bool flag = false;
	int external_object_num = 0;
	tag_t* external_object_list = NULLTAG;

	if (object_tag == NULL) {
		goto CLEANUP;
	}

	ITK_THROW_ERROR(AOM_ask_value_tags(object_tag, "IMAN_external_object_link", &external_object_num, &external_object_list));
	if (external_object_num > 0) {
		flag = true;
	}

CLEANUP:
	DOFREE(external_object_list);
	return flag;
}

int getHandlerArgs(TC_argument_list_t* arguments, map<string, string>& mp) {
	int arg_cnt = TC_number_of_arguments(arguments);
	for (int i = 0; i < arg_cnt; i++) {
		char* tmp_arg = TC_next_argument(arguments);
		char* arg_name = (char*)MEM_alloc(sizeof(char) * 256);
		memset(arg_name, 0, 256);
		strcpy(arg_name, "");

		char* arg_value = (char*)MEM_alloc(sizeof(char) * 256);
		memset(arg_value, 0, 256);
		strcpy(arg_value, "");

		ITK_THROW_ERROR(ITK_ask_argument_named_value(tmp_arg, &arg_name, &arg_value));
		TC_write_syslog("arg_name == %s, arg_value == %s\n", arg_name, U2G(arg_value));
		mp.insert(pair<string, string>(arg_name, arg_value));
		DOFREE(arg_name);
		DOFREE(arg_value);
	}
	return mp.max_size();
}

unsigned char ToHex(unsigned char x)
{
	return  x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x)
{
	unsigned char y;
	if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
	else if (x >= '0' && x <= '9') y = x - '0';
	else assert(0);
	return y;
}

std::string UrlEncode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		if (isalnum((unsigned char)str[i]) ||
			(str[i] == '-') ||
			(str[i] == '_') ||
			(str[i] == '.') ||
			(str[i] == '~'))
			strTemp += str[i];
		else if (str[i] == ' ')
			strTemp += "+";
		else
		{
			strTemp += '%';
			strTemp += ToHex((unsigned char)str[i] >> 4);
			strTemp += ToHex((unsigned char)str[i] % 16);
		}
	}
	return strTemp;
}

std::string UrlDecode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		if (str[i] == '+') strTemp += ' ';
		else if (str[i] == '%')
		{
			assert(i + 2 < length);
			unsigned char high = FromHex((unsigned char)str[++i]);
			unsigned char low = FromHex((unsigned char)str[++i]);
			strTemp += high * 16 + low;
		}
		else strTemp += str[i];
	}
	return strTemp;
}


/**
* 判断文件夹下是否含有某零组件
*/
boolean checkExistItem(tag_t primary_tag, tag_t second_tag, char* propName) {
	char
		* itemUID = NULL,
		* compareUID = NULL;

	tag_t
		* tContents = NULLTAG;
	int
		iCount = 0;

	boolean
		flag = false;

	AOM_refresh(primary_tag, false);

	AOM_ask_value_tags(primary_tag, propName, &iCount, &tContents);
	ITK__convert_tag_to_uid(second_tag, &itemUID);

	for (size_t i = 0; i < iCount; i++) {
		tag_t tContent = tContents[i];
		ITK__convert_tag_to_uid(tContent, &compareUID);
		if (tc_strcmp(itemUID, compareUID) == 0) {
			flag = true;
			break;
		}
	}
CLEANUP:
	DOFREE(tContents);
	DOFREE(itemUID);
	DOFREE(compareUID);
	return flag;
}

/**
* 更新属性
*/
int updateProp(tag_t item, const char* propName, const char* propValue) {
	int ifail = ITK_ok;
	try {
		ITK_THROW_ERROR(AOM_lock(item)) //锁定对象
		ITK_THROW_ERROR(AOM_load(item));
		ITK_THROW_ERROR(AOM_set_value_string(item, propName, propValue)); // 修改属性值
	} catch (int& error_code) {
		ifail = error_code;
	}	
	ITK_THROW_ERROR(AOM_save_with_extensions(item));
	ITK_THROW_ERROR(AOM_unlock(item)); //解锁对象
	return ifail;
}

/*
* 批量更新属性值
*/
int batchUpdataProp(tag_t item, map<string, string> propMap) {
	int ifail = ITK_ok;
	try {
		if (!propMap.empty()) {
			ITK_THROW_ERROR(AOM_lock(item)) //锁定对象
			ITK_THROW_ERROR(AOM_load(item));
			for (auto it = propMap.begin(); it != propMap.end(); it++) {
				ITK_THROW_ERROR(AOM_set_value_string(item, it->first.c_str(), it->second.c_str())); // 修改属性值
			}
		}		
		
	} catch (int& error_code) {
		ifail = error_code;
	}
	ITK_THROW_ERROR(AOM_save_with_extensions(item));
	ITK_THROW_ERROR(AOM_unlock(item)); //解锁对象
	return ifail;
}
/**
* 去除首尾空格
*/
string& clearHeadTailSpace(string& str) {
	if (str.empty())
	{
		return str;
	}

	str.erase(0, str.find_first_not_of(" "));
	str.erase(str.find_last_not_of(" ") + 1);
	return str;
}


/**
* 去掉字符串中的全部空格
*/
string& clearAllSpace(string& str) {
	int index = 0;
	if (!str.empty())
	{
		while ((index = str.find(' ', index)) != string::npos)
		{
			str.erase(index, 1);
		}
	}

	return str;
}


/*
* 字符串数字相乘
*/
string multiply(string num1, string num2) {
	string res = "";
	int m = num1.size(), n = num2.size();
	vector<int> vals(m + n);
	for (int i = m - 1; i >= 0; --i) {
		for (int j = n - 1; j >= 0; --j) {
			int mul = (num1[i] - '0') * (num2[j] - '0');
			int p1 = i + j, p2 = i + j + 1, sum = mul + vals[p2];
			vals[p1] += sum / 10;
			vals[p2] = sum % 10;
		}
	}
	for (int val : vals) {
		if (!res.empty() || val != 0) res.push_back(val + '0');
	}
	return res.empty() ? "0" : res;
}

char	logFileName[256];
FILE* logFile = NULL;

void createLogFile(char* FunctionName) {
	int i;
	date_t status_now;
	char* date_str = NULL;
	char logFileDir[MAX_PATH_LENGTH];
	char logFileName[MAX_PATH_LENGTH];

	logFile = NULL;

	memset(logFileDir, 0, sizeof(logFileDir));
	memset(logFileName, 0, sizeof(logFileName));
	//get log dir   add  rsq  修改环境变量
	sprintf(logFileDir, "%s%s%s", getenv("TEMP"), PATH_SEPARATOR, FunctionName);
	printf("log file dir: %s\n", logFileDir);
	//try to change dir to %TEMP%
	if (_chdir(logFileDir) != ITK_ok) {
		_mkdir(logFileDir); // 创建文件夹
	}

	current_time(&status_now);
	if (DATE_date_to_string(status_now, "%Y-%m-%d", &date_str) != ITK_ok) {
		printf("!*ERROR*!: Failed to get current date time\n");
		goto CLEANUP;
	}

	//get logFileName
	memset(logFileName, 0, sizeof(logFileName));
	sprintf(logFileName, "%s%s%s_%s.log", logFileDir, PATH_SEPARATOR, FunctionName, date_str);

	printf("logFileName=%s\n", logFileName);

	if (_access(logFileName, 0) == 0) {
		logFile = fopen(logFileName, "a+");
	}
	else {
		logFile = fopen(logFileName, "w");
	}

CLEANUP:
	DOFREE(date_str);
}

void writeLog(const char* format, ...) {
	va_list arg;
	char tmp[MAX_PRINTLINE_LENGTH];

	if (logFile != NULL) {
		if (format != NULL) {
			date_t status_now;
			char* date_str = NULL;

			current_time(&status_now);

			DATE_date_to_string(status_now, "%Y-%m-%d %H:%M:%S", &date_str); //精确到秒级

			//get the message
			memset(tmp, 0, sizeof(tmp));
			va_start(arg, format);
			vsnprintf(tmp, sizeof(tmp), format, arg);
			va_end(arg);

			//NLS_internal_to_external(tmp, &message);//UTF8编码中文乱码
//			printf("[%s]%s", date_str, tmp);

			if (logFile) {
				//print message to log file
				fprintf(logFile, "[%s] %s", date_str, tmp);
				fflush(logFile);
			}
			else {
				printf("*!Error!*: Log File Not Exist\n");
			}

		CLEANUP:
			DOFREE(date_str);
		}
	}
}

string getTcCode(const char* utf8) {
	char* real = NULL;
	NLS_internal_to_external(utf8, &real);
	string resStr = string(real);
CLEANUP:
	DOFREE(real);
	return resStr;
}

string getExternCode(const char* str) {
	char* real = NULL;
	NLS_external_to_internal(str, &real);
	string resStr = string(real);

CLEANUP:
	DOFREE(real);
	return resStr;
}

string getPropValue(tag_t obj, const char* propName) {
	string
		propValue = "";
	char
		* value = NULL;
	if (obj == NULLTAG) {
		printf("getPropValue obj tag==null,proprtry:%s\n", propName);
		return propValue;
	}

	AOM_ask_value_string(obj, propName, &value);

	
	if (NULL != value) {
//		propValue = getTcCode(value);
		propValue = string(value);
	}

CLEANUP:
	DOFREE(value);
	return propValue;
}

/*
* 获取当前登录用户
*/
string getCurUser() {
	tag_t
		member_tag = NULLTAG,
		user_tag = NULLTAG;

	ITK_THROW_ERROR(SA_ask_current_groupmember(&member_tag));
	ITK_THROW_ERROR(SA_ask_groupmember_user(member_tag, &user_tag));
	return getPropValue(user_tag, "user_id");
}

/*
* 获取当前登录用户所在的组
*/
string getCurGroup() {
	tag_t
		member_tag = NULLTAG,
		user_tag = NULLTAG,
		group_tag = NULLTAG;

	char
		* groupName = NULL;

	string
		groupNameValue;

	ITK_THROW_ERROR(SA_ask_current_groupmember(&member_tag));
	ITK_THROW_ERROR(SA_ask_groupmember_group(member_tag, &group_tag));
	ITK_THROW_ERROR(POM_ask_group_name(group_tag, &groupName));
	groupNameValue = (string)groupName;
CLEANUP:
	DOFREE(groupName);
	return groupNameValue;
}


char* createJsonFile(const char* FunctionName) {
	FILE* logFile = NULL;

	date_t status_now;
	char* date_str = NULL;
	char logFileDir[MAX_PATH_LENGTH];
	char logFileName[MAX_PATH_LENGTH];

	memset(logFileDir, 0, sizeof(logFileDir));
	memset(logFileName, 0, sizeof(logFileName));

	sprintf(logFileDir, "%s%s%s", getenv("TEMP"), PATH_SEPARATOR, FunctionName);

	if (_chdir(logFileDir) != ITK_ok) {
		_mkdir(logFileDir); // 创建文件夹
	}

	current_time(&status_now);
	if (DATE_date_to_string(status_now, "%Y-%m-%d", &date_str) != ITK_ok) {
		printf("!*ERROR*!: Failed to get current date time\n");
		goto CLEANUP;
	}

	//get logFileName
	memset(logFileName, 0, sizeof(logFileName));
	sprintf(logFileName, "%s%s%s_%s.json", logFileDir, PATH_SEPARATOR, FunctionName, date_str);

	printf("jsonFileName=%s\n", logFileName);

	if (_access(logFileName, 0) != 0) {
		logFile = fopen(logFileName, "a+");
		fclose(logFile);
	}


CLEANUP:
	DOFREE(date_str);
	return logFileName;
}

void appendJsonData(const char* fileName, const char* data) {
	FILE* fp;
	date_t status_now;
	char* date_str = NULL;

	if (_access(fileName, 0) == 0) { // 判断文件是否存在
		if ((fp = fopen(fileName, "a+")) != NULL) { // 选用追加模式
			current_time(&status_now);
			if (DATE_date_to_string(status_now, "%Y-%m-%d %H:%M:%S", &date_str) != ITK_ok) {
				printf("!*ERROR*!: Failed to get current date time\n");
				goto CLEANUP;
			}
			fprintf(fp, "%s %s\n", date_str, data); //将内容以字符串形式写入json文件
			fclose(fp); //关闭文件
		}
	}

CLEANUP:
	DOFREE(date_str);
}

/*
* 将string转换成wstring
*/
wstring string2wstring(string str) {
	wstring result;
	//获取缓冲区大小，并申请空间，缓冲区大小按字符计算  
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	TCHAR* buffer = new TCHAR[len + 1];
	//多字节编码转换成宽字节编码  
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
	buffer[len] = '\0';             //添加字符串结尾  
	//删除缓冲区并返回值  
	result.append(buffer);
	delete[] buffer;
	return result;
}


/*
* 将wstring转换成string
*/
string wstring2string(wstring wstr) {
	string result;
	//获取缓冲区大小，并申请空间，缓冲区大小事按字节计算的  
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
	char* buffer = new char[len + 1];
	//宽字节编码转换成多字节编码  
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
	buffer[len] = '\0';
	//删除缓冲区并返回值  
	result.append(buffer);
	delete[] buffer;
	return result;
}

std::string to_utf8(std::wstring& wide_string)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wide_string);
}

std::wstring from_utf8(std::string& string) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(string);
}

bool compareStrings(const std::string& str1, const std::string& str2) {
	return str1 < str2; // 使用字符串的字典序进行比较
}

/*
* 字符串全部大写
*/
std::string toUpperCase(const std::string& str) {
	std::string result = str;
	for (char& c : result) {
		c = std::toupper(c);
	}
	return result;
}

/*
* 字符串全部小写
*/
std::string toLowCase(const std::string& str) {
	std::string result = str;
	for (char& c : result) {
		c = std::tolower(c);
	}
	return result;
}

char* string2char(const std::string& str) {
	char* cstr = new char[str.length() + 1];
	std::strcpy(cstr, str.c_str());

}

void unpackBomLine(tag_t bomLine) {
	if (bomLine != NULL) {
		tag_t* childrens;
		tag_t childBOMLine;
		int count = 0;
		ITK_THROW_ERROR(BOM_line_ask_all_child_lines(bomLine, &count, &childrens));
		if (count > 0) {
			for (int i = 0; i < count; i++) {
				childBOMLine = childrens[i];
				unpackBomLine(childBOMLine);
			}
		}
		ITK_THROW_ERROR(BOM_line_unpack(bomLine));
	}
}

/*
* 判断vector中的某一元素是否存在
*/
bool is_element_in_vector(vector<string>& v, string str) {
	vector<string>::iterator it;
	it = find(v.begin(), v.end(), str);
	if (it != v.end()) {
		return true;
	}
	return false;
}

/*
* 返回vector中的某个元素的下标志
*/
int element_index(vector<string>& v, string str) {
	int res = -1;
	vector<string>::iterator it;
	it = find(v.begin(), v.end(), str);
	if (it != v.end()) {
		res = it - v.begin();
	}
	return res;
}

/*
* 返回以某字符开头的元素
*/
string getElementStartWith(vector<string>& v, string str) {
	string value = "";
	for (auto it = v.begin(); it != v.end();) {
		if (startWithStr(*it, str)) {
			value = *it;
			break;
		} else {
			it++;
		}
	}
	return value;
}

/*
* 返回以某字符结束的元素
*/
string getElementEndWith(vector<string>& v, string str) {
	string value = "";
	for (auto it = v.begin(); it != v.end();) {
		if (endWithStr(*it, str)) {
			value = *it;
			break;
		} else {
			it++;
		}
	}
	return value;
}

/*
* 将对象指针转换为集合数组
*/
vector<tag_t> getTagList(tag_t* list, int num) {
	vector<tag_t> tagList;
	for (int i = 0; i < num; i++) {
		tagList.push_back(list[i]);
	}
	return tagList;
}

/*
* 返回当前对象指派的所有专案
*/
string getCurProjectId(tag_t item) {
	int
		num = 0;

	string
		projectIdStr = "";

	char
		* projectId = NULL,
		* projectName = NULL;

	tag_t
		project = NULLTAG;

	tag_t*
		projects = NULLTAG;

	ITK_THROW_ERROR(AOM_ask_value_tags(item, project_list, &num, &projects));
	for (int i = 0; i < num; i++) {
		project = projects[i];

		ITK_THROW_ERROR(PROJ_ask_id2(project, &projectId));
		writeLog("curProjectId == %s\n", projectId);

		ITK_THROW_ERROR(PROJ_ask_name2(project, &projectName));
		writeLog("curprojectName == %s\n", projectName);

		if (i == num - 1) {
			projectIdStr += (string)projectId;
		} else {
			projectIdStr += (string)projectId + ",";
		}
	}

CLEANUP:
	DOFREE(projectId);
	DOFREE(projectName);
	return toUpperCase(projectIdStr);
}


/*
* 查询根据item返回零组件集合
*/
vector<tag_t> queryItems(string item) {
	vector<tag_t> itemList;
	int
		resultLength = 0;		

	tag_t
		* queryResults = NULLTAG;

	logical
		isItem = false;

	char** qry_entries = (char**)MEM_alloc(sizeof(char**));
	char** qry_values = (char**)MEM_alloc(sizeof(char**));
	qry_entries[0] = (char*)MEM_alloc(sizeof(char*) * 200);
	qry_values[0] = (char*)MEM_alloc(sizeof(char*) * 200);

	tc_strcpy(qry_entries[0], ITEM_ID);
	tc_strcpy(qry_values[0], item.c_str());

	ITK_THROW_ERROR(query(D9_ITEMID, qry_entries, qry_values, 1, &queryResults, &resultLength));
	writeLog(" -- >> D9_MaterialGroupRevision find : %s queryresult : %d \n", item, resultLength);
	for (int i = 0; i < resultLength; i++) {
		ITK_THROW_ERROR(GTCTYPE_is_type_of(queryResults[i], "Item", &isItem));
		if (!isItem) {
			continue;
		}

		itemList.push_back(queryResults[i]);
	}

CLEANUP:
	DOFREE(qry_entries);
	DOFREE(qry_values);
	DOFREE(queryResults);
	return itemList;
}


/*
* 通过零组件属性查找零组件
*/
vector<tag_t> findItemsByAtt(vector<string> attNames, vector <string> attValue) {
	vector<tag_t> itemList;
	int 
		count = 0;

	tag_t
		* items = NULLTAG;

	const char** attrs = new const char* [attNames.size() + 1];
	const char** values = new const char* [attValue.size() + 1];
	for (size_t i = 0; i < attNames.size(); i++) {
		attrs[i] = attNames[i].c_str();
	}

	for (size_t j = 0; j < attValue.size(); j++) {
		values[j] = attValue[j].c_str();
	}

	ITK_THROW_ERROR(ITEM_find_items_by_key_attributes(1, attrs, values, &count, &items));
	for (int k = 0; k < count; k++) {
		itemList.push_back(items[k]);
	}

CLEANUP:
	DOFREE(items);
	if (attrs != NULL) {
		delete[] attrs;
		attrs = NULL;
	}
	if (values != NULL) {
		delete[] values;
		values = NULL;
	}
	return itemList;
}

static std::tr1::regex numberPattern("[0-9]+"); // 匹配是否全部为数字
static std::tr1::regex letterPattern("[a-zA-Z]+"); // 匹配是否全部为字母
/*
* 获取版本规则
*/
string getVersionRule(tag_t itemRev) {
	string versionRule = "";
	string version = getPropValue(itemRev, current_revision_id);
	if (regex_matchstr(version, numberPattern)) { // 判断对象版本是否为数字版
		versionRule = NumPattern;
	} else if (regex_matchstr(version, letterPattern)) { // 判断是否为字母版
		versionRule = LetterPattern;
	}
	return versionRule;
}


/*
* 传递对象类型,升版模式
*/
string getVersionByRule(tag_t item, string pattern) {
	string 
		newRev;
	char
		* newItemId = NULL,
		* itemType = NULL;
	ITK_THROW_ERROR(WSOM_ask_object_type2(item, &itemType)); // 获取对象类型
	ITK_THROW_ERROR(NR_pattern_next_value(itemType, item_revision_id, item, "", "", "", NULLTAG, "", "", pattern.c_str(), &newItemId)); // 通过对象和升版模式获取下一个版本的版本号
	newRev = (string)newItemId;
CLEANUP:
	DOFREE(itemType);
	DOFREE(newItemId);
	return newRev;
}


/*
* 对象升版
*/
int reviseRev(tag_t basedRev, string newRevId, tag_t* newRev) {
	int 
		ifail = ITK_ok;
	try {
		ITK_THROW_ERROR(AOM_refresh(basedRev, false));
		ITK_THROW_ERROR(AOM_lock(basedRev)) //锁定版本
		ITK_THROW_ERROR(AOM_load(basedRev));
		ITK_THROW_ERROR(ITEM_copy_rev(basedRev, newRevId.c_str(), newRev));
		ITK_THROW_ERROR(AOM_save_with_extensions(basedRev));
		ITK_THROW_ERROR(AOM_unlock(basedRev)); //解锁版本
		ITK_THROW_ERROR(AOM_refresh(basedRev, false));
	} catch (int& error_code) {
		ifail = error_code;
	}
	return ifail;
}


/*
* 创建零组件
*/
int createItem(string itemId, string itemName, string itemType, string pattern, tag_t* newItem, tag_t* newItemRev) {
	int
		ifail = ITK_ok;
	string
		newRev;
	char
		* newItemId = NULL;
	try {
		ITK_THROW_ERROR(NR_pattern_next_value(itemType.c_str(), item_revision_id, NULLTAG, "", "", "", NULLTAG, "", "", pattern.c_str(), &newItemId));
		newRev = (string)newItemId;
		ITK_THROW_ERROR(ITEM_create_item(itemId.c_str(), itemName.c_str(), itemType.c_str(), newRev.c_str(), newItem, newItemRev));
		ITK_THROW_ERROR(AOM_lock(*newItem));
		ITK_THROW_ERROR(AOM_lock(*newItemRev));
		ITK_THROW_ERROR(ITEM_save_item(*newItem));
		ITK_THROW_ERROR(ITEM_save_rev(*newItemRev));
		ITK_THROW_ERROR(AOM_unlock(*newItem));
		ITK_THROW_ERROR(AOM_unlock(*newItemRev));
	} catch (int& error_code) {
		ifail = error_code;
	}
	
CLEANUP:
	DOFREE(newItemId);
	return ifail;
}

void sortVectorByName(vector<string>& list) {
	//把函数名当成第三个参数传递进去，效果是一模一样的
	std::sort(list.begin(), list.end()
		, [](const string& a, string& b) {return a < b; });
}


/*
* 对vector数组按照制定大小分组
*/
std::vector<std::vector<string>> groupVector(const std::vector<string>& vec, int groupSize) {
	std::vector<std::vector<string>> groupedVec;
	int numGroups = vec.size() / groupSize;
	int remaining = vec.size() % groupSize;

	int startIndex = 0;
	for (int i = 0; i < numGroups; i++) {
		std::vector<string> group;
		for (int j = 0; j < groupSize; j++) {
			group.push_back(vec[startIndex + j]);
		}
		groupedVec.push_back(group);
		startIndex += groupSize;
	}

	if (remaining > 0) {
		std::vector<string> group;
		for (int i = 0; i < remaining; i++) {
			group.push_back(vec[startIndex + i]);
		}
		groupedVec.push_back(group);
	}

	return groupedVec;
}

/*
* 去重
*/
void distinctValue(vector<string>& list) {
	sortVectorByName(list); // 排序
	list.erase(std::unique(list.begin(), list.end(), [](const string& str1, const string& str2) {
		return str1.compare(str2) == 0;
	}), list.end());
}

/*
 计算数值型字符串的小数位数
*/
int getDecimalPlaces(std::string num) {
	size_t decimalPos = num.find('.');
	if (decimalPos != std::string::npos) {
		return num.length() - decimalPos - 1;
	}
	return 0;
}

/**
* 两数相加，支持浮点数不丢精度
*/
string add(string x, string y) {
	//std::cout << "x is: " << x << std::endl;
	//std::cout << "y is: " << y << std::endl;
	if (x.empty()) {
		x = "0";
	}

	if (y.empty()) {
		y = "0";
	}
	int scale1 = getDecimalPlaces(x);
	int scale2 = getDecimalPlaces(y);
	int maxScale = scale1 > scale2 ? scale1 : scale2;
	//std::cout << "maxScale is: " << maxScale << std::endl;

	double xd = std::stod(x) * pow(10, maxScale); // 转换为double  
	double yd = std::stod(y) * pow(10, maxScale); // 转换为double  
	double xyd = xd + yd;
	double xy_scaled = xyd * pow(10, -maxScale);

	//std::cout << "xd is: " << xd << std::endl;
	//std::cout << "yd is: " << yd << std::endl;
	//std::cout << "xyd is: " << xyd << std::endl;
	//std::cout << "x+y is: " << xy_scaled << std::endl;

	std::stringstream ss;
	ss << std::setprecision(maxScale + 1) << xy_scaled;
	string result = ss.str();
	//std::cout << "result is: "  << result << std::endl;
	return result;
}

/*
* 获取TC报错信息
*/
string getErrorMsg(int ifail) {
	string msg = "";
	if (ifail != ITK_ok) {
		int n_ifails = 0;                                          
		const int* severities = NULL;                                     
		const int* ifails = NULL;                                      
		const char** texts = NULL;                                      
		EMH_ask_errors(&n_ifails, &severities, &ifails, &texts);
		if (n_ifails > 0) {
			if (texts[n_ifails - 1]) {
				msg += texts[n_ifails - 1];
			}
		}
	}

	return msg;
}


/*
* 判断是否是最新版
*/
bool isNewRevision(tag_t itemRev) {
	int
		count = 0;
	tag_t
		item = NULLTAG;
	tag_t	
		* revList = NULLTAG;
	ITK_THROW_ERROR(ITEM_ask_item_of_rev(itemRev, &item)); // 对象版本获取对象
	ITK_THROW_ERROR(ITEM_list_all_revs(item, &count, &revList)); // 获取所有的版本对象
	vector<tag_t> itemRevList = getTagList(revList, count); // 将指针集合转换为vector	
	if (itemRev == itemRevList[itemRevList.size() - 1]) {
		return true;
	}

	return false;
}

void createBomView(tag_t& item, tag_t& item_rev) {
	int num;
	tag_t* values = NULL;
	char* view_name = NULL;
	ITKCALL(AOM_ask_value_tags(item, "bom_view_tags", &num, &values));
	if (num == 0)
	{
		tag_t view_type = NULLTAG,
			bom_view = NULLTAG,
			bvr = NULLTAG;
		ITKCALL(AOM_lock(item));
		ITKCALL(PS_ask_default_view_type(&view_type));
		ITKCALL(PS_default_bom_view_name(item, view_type, &view_name));
		ITKCALL(PS_create_bom_view(view_type, NULL, NULL, item, &bom_view));
		ITKCALL(AOM_save_with_extensions(bom_view));
		ITKCALL(AOM_save_with_extensions(item));
		ITKCALL(AOM_unlock(item));
		ITKCALL(AOM_lock(item_rev));
		ITKCALL(PS_create_bvr(bom_view, "", "", FALSE, item_rev, &bvr));
		ITKCALL(AOM_save_with_extensions(bvr));
		ITKCALL(AOM_save_with_extensions(item_rev));
		ITKCALL(AOM_unlock(bom_view));
		ITKCALL(AOM_unlock(item_rev));
	}
	else
	{
		int rev_num = 0;
		tag_t* rev_values = NULL;
		ITKCALL(AOM_ask_value_tags(item_rev, "structure_revisions", &rev_num, &rev_values));
		if (rev_num == 0)
		{
			tag_t bvr = NULLTAG;
			ITKCALL(AOM_lock(item_rev));
			ITKCALL(PS_create_bvr(values[0], "", "", FALSE, item_rev, &bvr));
			ITKCALL(AOM_save_with_extensions(bvr));
			ITKCALL(AOM_save_with_extensions(item_rev));
			ITKCALL(AOM_unlock(item_rev));
		}
		DOFREE(rev_values);
	}

	DOFREE(values);
	DOFREE(view_name);
}

