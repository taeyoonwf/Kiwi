#include <cstdio>
#include <tchar.h>
#include <windows.h>
#include <locale.h>

#include <kiwi/capi.h>

int reader(int id, kchar16_t* buf, void* user)
{
	if (id >= 10)
	{
		return 0;
	}
	if (buf == nullptr) return 20;
	swprintf_s((wchar_t*)buf, 20, L"테스트 %d입니다.", id);
}

int receiver(int id, kiwi_res_h kr, void* user)
{
	int size = kiwi_res_size(kr);
	for (int i = 0; i < size; i++)
	{
		printf("%g\t", kiwi_res_prob(kr, i));
		int num = kiwi_res_word_num(kr, i);
		for (int j = 0; j < num; j++)
		{
			wprintf(L"%s/%s(%d~%d)\t", kiwi_res_form_w(kr, i, j), kiwi_res_tag_w(kr, i, j),
				kiwi_res_position(kr, i, j), kiwi_res_position(kr, i, j) + kiwi_res_length(kr, i, j));
		}
		printf("\n");
	}

	kiwi_res_close(kr);
	return 0;
}

int main()
{
	system("chcp 65001");
	_wsetlocale(LC_ALL, L"korean");
	for (size_t t = 0; t < 10; ++t)
	{
		kiwi_h kw = kiwi_init("ModelGenerator/", 0, 0);
		kiwi_prepare(kw);
		kiwi_analyze_mw(kw, reader, receiver, nullptr, 10);
		kiwi_res_h kr;
		FILE* f = fopen("test/sample/longText.txt", "r");
		fseek(f, 0, SEEK_END);
		size_t len = ftell(f);
		kchar16_t* longText = (kchar16_t*)malloc(len);
		fseek(f, 0, SEEK_SET);
		fread(longText, 1, len, f);
		fclose(f);
		kr = kiwi_analyze_w(kw, (const kchar16_t*)longText, 1);
		free(longText);
		int size = kiwi_res_size(kr);
		for (int i = 0; i < size; i++)
		{
			printf("%g\t", kiwi_res_prob(kr, i));
			int num = kiwi_res_word_num(kr, i);
			for (int j = 0; j < num && j < 1000; j++)
			{
				wprintf(L"%s/%s(%d~%d)\t", kiwi_res_form_w(kr, i, j), kiwi_res_tag_w(kr, i, j),
					kiwi_res_position(kr, i, j), kiwi_res_position(kr, i, j) + kiwi_res_length(kr, i, j));
			}
			printf("\n");
		}
		kiwi_res_close(kr);
		kiwi_close(kw);
	}
	getchar();
    return 0;
}

