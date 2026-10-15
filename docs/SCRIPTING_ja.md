# InstallZone スクリプト作成ガイド (日本語)

InstallZone は、Duktape をベースとした軽量で安全な組み込み JavaScript エンジンを搭載しています。これにより、インストーラー実行時に高度なシステム設定、`~/.config` 配下のフォルダ自動作成、環境変数の設定、`navi` などの依存ツールの設定ファイルを自動生成できます。

---

## 1. 実行ライフサイクルとフック (Hooks)

InstallZone では、以下の 4 つの JavaScript フックを提供しています：

1. **インストール前フック (`pre_install_js`)**:
   - ファイルの展開前に実行されます。
   - 用途: 実行中プロセスの停止、カスタム事前確認、既存ファイルや設定のバックアップ。

2. **インストール後フック (`post_install_js`)**:
   - ファイル展開およびデスクトップエントリーの作成後に実行されます。
   - 用途: ユーザー設定フォルダ (`~/.config/<アプリ名>`) の作成、`navi` チートシートの登録、シェル補完設定、シンボリックリンクの作成。

3. **アンインストール前フック (`pre_uninstall_js`)**:
   - ファイル削除前に実行されます。
   - 用途: サービスの停止、登録解除処理。

4. **アンインストール後フック (`post_uninstall_js`)**:
   - ファイルおよびショートカットの削除完了後に実行されます。
   - 用途: 一時ファイルのクリーンアップ。

---

## 2. グローバルオブジェクト: `iz`

JavaScript コンテキストには、以下のプロパティおよび関数を持つ `iz` オブジェクトが自動的に注入されます。

### プロパティ一覧

| プロパティ名 | 型 | 説明 |
| :--- | :--- | :--- |
| `iz.app_name` | `string` | アプリケーションの表示名。 |
| `iz.version` | `string` | バージョン文字列 (例: `"1.0.0"`). |
| `iz.install_dir` | `string` | インストール先ディレクトリの絶対パス (例: `"/opt/myapp"`). |
| `iz.home_dir` | `string` | 実際のデスクトップユーザーのホームディレクトリ (例: `"/home/username"`). `sudo` や `pkexec` で実行された場合でも正しく一般ユーザーのパスを検出します。 |
| `iz.config_dir` | `string` | 設定フォルダのパス (`~/.config`). |
| `iz.is_uninstall` | `boolean` | アンインストール処理中は `true`、インストール中は `false`。 |

---

### メソッド一覧

#### `iz.log(message)`
インストーラー画面のリアルタイムログ出力欄およびログファイルにメッセージを出力します。
```javascript
iz.log("インストール後の初期設定を開始します...");
```

#### `iz.mkdir(path)`
指定されたパスにディレクトリを再帰的に作成します (`mkdir -p`)。パス先頭のチルダ (`~/`) は自動的にユーザーのホームディレクトリに展開されます。ルート権限で実行中の場合、作成されたフォルダの所有権は自動的に実ユーザーへ設定されます。
```javascript
var cfgDir = iz.config_dir + "/myapp";
iz.mkdir(cfgDir);
```

#### `iz.write_file(path, content)`
指定されたパスに文字列データをファイルとして書き込みます。親ディレクトリが存在しない場合は自動作成します。
```javascript
var configFile = iz.config_dir + "/myapp/settings.json";
iz.write_file(configFile, JSON.stringify({ theme: "dark", lang: "ja" }, null, 2));
```

#### `iz.read_file(path)`
テキストファイルの内容を文字列として読み込みます。開けない場合は `null` を返します。

#### `iz.file_exists(path)`
ファイルまたはディレクトリが存在するかどうかを返します (真偽値)。

#### `iz.remove(path)`
指定されたファイルまたは空ディレクトリを削除します。

#### `iz.copy(src, dst)`
ファイルをコピーします。

#### `iz.exec(command)`
シェルコマンドを実行し、終了コード (`exit_code`) および標準出力 (`stdout`) を含むオブジェクトを返します。
```javascript
var res = iz.exec("which navi");
if (res.exit_code === 0) {
    iz.log("navi コマンドが検出されました: " + res.stdout.trim());
}
```

---

## 3. 実践例: Navi チートシートの設定

アプリケーションが `navi` のようなツールに依存する場合、`post_install_js` で以下のように自動設定できます：

```javascript
iz.log(iz.app_name + " 用の Navi チートシートを設定中...");

var naviDir = iz.home_dir + "/.config/navi/cheats";
iz.mkdir(naviDir);

var cheatContent = 
    "% " + iz.app_name + "\n" +
    "# " + iz.app_name + " を詳細ログ付きで起動\n" +
    iz.app_name.toLowerCase() + " --verbose\n\n" +
    "# 設定ファイルを初期化\n" +
    iz.app_name.toLowerCase() + " --reset\n";

iz.write_file(naviDir + "/" + iz.app_name.toLowerCase() + ".cheat", cheatContent);
iz.log("Navi の設定が完了しました！");
```
