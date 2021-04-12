// 住信SBIネット銀行の取引csvを、やよいの青色申告に取り込めるcsvに変換
// 参考ページ：http://tax.f-blog.org/yayoi/CSV-Import.html
//
// Visual C++にてコンソールアプリケーションとしてコンパイルする。
//

#include <iostream>
#include <fstream>
#include <boost/tokenizer.hpp>

// ユーザー定義の例外
class MyException : public std::runtime_error
{
public:
	explicit MyException(const std::string& msg) : std::runtime_error(msg) {}
	virtual ~MyException() throw() {}
};

// ユーザー定義の例外（使用法を表示して終了させたいときに使用）
class ShowUsageException : public std::runtime_error
{
public:
	ShowUsageException() : std::runtime_error("") {}
	//explicit ShowUsageException(const std::string& msg) : std::runtime_error(msg) {}
	virtual ~ShowUsageException() throw() {}
};

int main(int argc, char* argv[])
{
	try {
		// コマンドライン引数の数をチェック
		if (argc == 3)
		{
			;
		}
		else
		{
			throw ShowUsageException();
		}

		// 入力ファイルopen
		std::ifstream fi;
		fi.open(argv[1]);
		if (fi) {
			;
		}
		else {
			throw MyException("could not open input file");
		}

		std::cout << "input_csvの情報\n";

		// タブ区切りCSVのread
		std::vector<std::string> header;
		std::vector<std::vector<std::string> > records;

		std::cout << "\t項目：\n";
		std::string line;
		while (getline(fi, line))
		{
			boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
			boost::tokenizer<boost::char_separator<char>, std::string::const_iterator, std::string> tokens(line, sep);
			if (header.empty())
			{
				for (const auto& token : tokens)
				{
					header.push_back(token);
					std::cout << "\t\t(" << header.size() << ") " << token << '\n';
				}
				if (!header.empty())
				{
					;
				}
				else
				{
					throw MyException("header is empty");
				}
			}
			else
			{
				records.push_back(std::vector<std::string>());
				for (const auto& token : tokens)
				{
					if (token.empty() || token[0] == '"') {
						records.back().push_back(token);
					}
					else {
						// 先頭がダブルクォーテーションじゃない時は、前回のトークンの続きとみなす
						records.back().back() += ("," + token);
					}
				}
				if (records.back().size() == header.size())
				{
					;
				}
				else
				{
					std::cout << records.size() << '\t' << records.back().size() << '\n';
					throw MyException("token num mismatch");
				}
			}
		}
		std::cout << "\t項目数：" << header.size() << '\n';
		std::cout << "\tレコード数：" << records.size() << '\n';

		// 出力ファイルopen
		std::ofstream fo;
		fo.open(argv[2]);
		if (fo) {
			;
		}
		else {
			throw MyException("could not open output file");
		}

		// write
		fo << R"(#,"2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25")" << '\n';
		fo << R"(#,"伝票NO","決算","取引日付","借方勘定科目","借方補助科目","借方部門","借方税区分","借方金額","借方税金額","貸方勘定科目","貸方補助科目","貸方部門","貸方税区分","貸方金額","貸方税金額","摘要","番号","期日","タイプ","生成元","仕訳メモ","付箋1","付箋2","調整")" << '\n';


		//for (const auto& record : records)
		for (auto itr = records.rbegin(); itr != records.rend(); itr++)
		{
			const auto& record(*itr);

			bool shiharai = true;
			std::string kingaku;
			if (!record[2].empty() && record[3].empty()) {
				kingaku = record[2];
			}
			else if (record[2].empty() && !record[3].empty()) {
				shiharai = false;
				kingaku = record[3];
			}
			else {
				throw MyException("出金金額(円)or入金金額(円)の長さがおかしい");
			}

			// 1 識別フラグ 必須
			// 仕訳の識別番号を半角数字で記述。
			// 「伝票以外の仕訳データ」は、2000となっています。
			// 「仕訳日記帳」に取り込むので、2000になります。
			fo << R"("2000",)";

			// 2 伝票NO
			// 空白にしておくと、自動的に割り振ってくれるので空白のままで。
			fo << ',';

			// 3 決算
			// 仕訳が決算仕訳の場合は「中決」「本決」を記述。通常の仕訳の場合は、空白でOKです。
			fo << ',';

			// 4 取引日付 必須
			// 会計期間内である必要があります。
			// 2015年7月2日の場合、「20150702」「2015/07/02」「2015/7/2」「H27/07/02」「H27/7/2」の
			// 形式で記入されている必要があります。
			fo << record[0] << ',';

			// 5 借方勘定科目　必須
			// やよいの青色申告で登録されている勘定科目か、1字違いのようなものでも
			// マッチング処理で取り込むことができます。
			// クレジットカードで経費を購入した場合、「消耗品費」に。携帯電話代の場合は
			// 「通信費」にしておく。
			// 今まで入力してきた仕訳日記帳を参考にすれば、すぐにわかると思います。
			if (shiharai) {
				fo << R"("事業主貸",)";
			}
			else {
				fo << R"("普通預金",)";
			}

			// 6 借方補助科目
			// 補助科目を登録していて、その補助科目を使いたい場合は、ここに指定。
			// 例えば、「借方勘定科目」の「通信費」に、「携帯電話代」を登録していて、
			// これを使いたかった場合とか。
			// 参照:携帯電話代を経費で落とす
			if (shiharai) {
				fo << R"("",)";
			}
			else {
				fo << R"("住信SBIネット銀行",)";
			}

			// 7 借方部門
			// 空白で。
			fo << ',';

			// 8 借方税区分 必須
			// 消費税の課税事業者でない場合は、「対象外」で。
			fo << R"("対象外",)";

			// 9 借方金額 必須
			// 金額。「 , 」は必要ありません。
			// 「 , 」がある場合は、「 "（ダブルクォーテーション）」で囲みます。
			fo << kingaku << ',';

			// 10 借方税金額
			// 課税事業者でなければ、空白で。
			fo << ',';

			// 11 貸方勘定科目 必須
			// やよいの青色申告で登録されている勘定科目を記入。
			// クレジットカードで消耗品など経費で計上するものを購入した場合、
			// 「未払金」にしておきます。
			// 銀行の明細の場合は、ここは「普通預金」になるでしょう。
			if (shiharai) {
				fo << R"("普通預金",)";
			}
			else {
				fo << R"("事業主借",)";
			}

			// 12 貸方補助科目
			// 補助科目を登録していてそれを指定する場合は、記入。
			// 例えば普通預金の明細を取り込む場合は、「三菱東京UFJ銀行」とか
			// 「ジャパンネット銀行」とか登録してある補助科目になるわけです。
			// 補助科目にかんしては、補助科目の作成を参考にしてください。
			if (shiharai) {
				fo << R"("住信SBIネット銀行",)";
			}
			else {
				fo << R"("",)";
			}

			// 13 貸方部門
			// 空白で。
			fo << ',';

			// 14 貸方税区分
			// 課税事業者でなければ、空白で。
			fo << R"("対象外",)";

			// 15 貸方金額 必須
			// 基本的には「借方金額」と同額を入力。
			fo << kingaku << ',';

			// 16 貸方税金額
			// 課税事業者でなければ、空白で。
			fo << ',';

			// 17 摘要
			// クレジットカードや銀行の明細には、たいてい摘要に該当するものが
			// 記載されているので、それを記入しておくと後で便利です。
			fo << record[1] << ',';

			// 18 番号
			// 未記入で。
			fo << ',';

			// 19 期日
			// 未記入で。
			fo << ',';

			// 20 タイプ 必須
			// 0：仕訳データ
			// 1：出金伝票データ
			// 2：入金伝票データ
			// 3：振替伝票データ
			// クレジットカードや銀行の明細をインポートする場合は、「 0 」で。
			fo << R"("0",)";

			// 21 生成元
			// 未記入で。
			fo << ',';

			// 22 仕訳メモ
			// 未記入で。
			fo << ',';

			// 23 付箋1
			// 未記入で。
			fo << ',';

			// 24 付箋2
			// 未記入で。
			fo << ',';

			// 25 調整
			// 調整にチェックを付ける場合は、「yes」を。そうでなければ「no」。
			// 基本的には「no」で。
			fo << R"("no")" << '\n';
		}
		std::cout << "完了しました" << std::endl;
	}
	catch (ShowUsageException& ex) {
		if (ex.what()[0]) {
			std::cerr << "Error: " << ex.what() << '\n' << std::endl;
		}
		std::cout << "csv_sbi2yayoi Version 1.0\n";
		std::cout << "Usage: csv_sbi2yayoi in_csv out_csv" << std::endl;
		return 1;
	}
	catch (std::exception& ex) {
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}
	return 0;
}

