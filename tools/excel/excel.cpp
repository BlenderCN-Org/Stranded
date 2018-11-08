#include "BasicExcelVC6.hpp"
#include <string>

using namespace YExcel;

bool set_wstring(string& str, BasicExcelCell *cell)
{
	if(cell)
	{
		size_t length = str.size();
		wchar_t *wch = new wchar_t[length + 1];
		mbstowcs(wch, str.c_str(), length);
		wch[length] = '\0';

		cell->SetWString(wch);

		delete[] wch;
		wch = NULL;

		return true;
	}

	return false;
}

bool get_wstring(string& wstr, BasicExcelCell *cell)
{
	if(cell)
	{
		if(cell->Type() == BasicExcelCell::WSTRING)
		{
			size_t wlength = wcslen(cell->GetWString());
			char *temp = new char[wlength * 3 + 1];
			memset(temp, '\0', wlength + 1);
			setlocale(LC_ALL, "");	//设置代码页
			wcstombs(temp, cell->GetWString(), wlength * 3);
			temp[wlength * 3] = '\0';

			wstr = temp;

			delete[] temp;
			temp = NULL;

			return true;
		}
	}
	return false;
}

//查找第三行以后，第一个为空的行
bool is_null_row(BasicExcelCell *cell)
{
	if(cell)
	{
		if(cell->Type() != BasicExcelCell::UNDEFINED)
			return false;
	}
	return true;
}

bool is_total_row(BasicExcelCell *cell)
{
	if(cell)
	{
		string temp_str = "";
		if(get_wstring(temp_str, cell))
		{
			size_t found1 = temp_str.find("合计");
			size_t found2 = temp_str.find("总计");
			if(found1 != string::npos || found2 != string::npos)
				return true;
		}
	}
	return false;
}

bool get_month(string& month, BasicExcelCell *cell)
{
	if(cell)
	{
		if(get_wstring(month, cell))
		{
			size_t found = month.find("月");
			if(found != string::npos)
			{
				month = month.substr(0, found + 2);
				if(!month.empty())
					return true;
			}
		}		
	}
	return false;
}

bool set_cell_title(BasicExcelWorksheet *sheet, size_t row, vector<string>& title_vec)
{
	if(sheet && !title_vec.empty())
	{
		for (vector<string>::size_type sx = 0; sx != title_vec.size(); ++sx)
		{
			BasicExcelCell *cell = sheet->Cell(row, sx);
			if(cell)
			{
				set_wstring(title_vec[sx], cell);
			}
		}
		return true;
	}
	return false;
}

int main(int argc, char* argv[])
{
	BasicExcel excel;

	vector<string> title_vec = vector<string>(0);
	string month = "";

	BasicExcel new_excel;
	new_excel.New(1);
	BasicExcelWorksheet *new_sheet = new_excel.GetWorksheet("Sheet1");
	BasicExcelCell * new_cell = NULL;

	string infile = "";
	string outfile = "";

	cout << "请输入导入EXCEL路径(E:\\excel\\excel\\Debug\\1.xls):" << endl;
	cin >> infile;

	cout << "请输入生成EXCEL路径(E:\\excel\\excel\\Debug\\2.xls):" << endl;
	cin >> outfile;

	excel.Load(infile.c_str());
	BasicExcelWorksheet *sheet1 = excel.GetWorksheet("Sheet1");
	if(sheet1)
	{
		bool work_line = false;
		size_t row = 0;
		size_t col = 0;
		size_t max_rows = sheet1->GetTotalRows();
		size_t max_cols = sheet1->GetTotalCols();
		
		for (size_t r = 1; r < max_rows - 1; ++r)
		{
			if(is_total_row(sheet1->Cell(r, 0)))
				continue;

			if(r > 2)
			{
				//去除多余的行
				if(is_null_row(sheet1->Cell(r, 0)))
					continue;

				//每行的列头[titl_vec由下面获取]
				if(set_cell_title(new_sheet, row, title_vec))
				{
					row ++;
				}
			}

			for (size_t c = 0; c < max_cols; ++c)
			{
				string temp_str = "";
				BasicExcelCell *cell = sheet1->Cell(r, c);
				if(r == 1)
				{
					//第2行获取月份
					if(month.empty())
						get_month(month, cell);
				}
				else if(r == 2)
				{
					//第3行获取列头
					temp_str = "";
					if(get_wstring(temp_str, cell))
					{
						if(temp_str != "")
							title_vec.push_back(temp_str);
					}
				}
				else
				{
					if(new_sheet)
					{
						//去除多余的列
						if(c >= title_vec.size())
							break;

						new_cell = new_sheet->Cell(row, c);
						if(new_cell)
						{
							if(c == 0)
							{
								set_wstring(month, new_cell);
							}
							else if(c == 1)
							{
								temp_str = "";
								if(get_wstring(temp_str, cell))
								{
									if(set_wstring(temp_str, new_cell))
										work_line = true;
								}
							}
							else
							{
								temp_str = "";
								switch (cell->Type())
								{
								case BasicExcelCell::INT:
									new_cell->SetInteger(cell->GetInteger());
									break;
								case BasicExcelCell::DOUBLE:
									new_cell->SetDouble(cell->GetDouble());
									break;
								case BasicExcelCell::STRING:
									new_cell->SetString(cell->GetString());
									break;
								case BasicExcelCell::WSTRING:
									if(get_wstring(temp_str, cell))
									{
										if(temp_str != "")
											set_wstring(temp_str, new_cell);
									}
									break;
								default:
									break;
								}
							}
						}
					}
				}
			}

			if(r > 2 && work_line)
			{
				row ++;
				work_line = false;
			}
		}

		new_excel.SaveAs(outfile.c_str());
	}

	return 0;
}