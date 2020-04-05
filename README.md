# kyuubi

## ��������

`kyuubi` ��һ������ `C++` ʵ�ֵ��ַ���ģ��⡣�������� `xiyue-studio` �� `xiyue` �����⡣

�û�ֻ��Ҫ������滻ģ�壬�Լ��ṩ����Ҫ���� `json` ���ݣ��Ϳ����Զ�����Ŀ���ļ���
���磬���������� `json` ���ݣ�

```json
{
	"name": "Tecyle",
	"age": 18,
	"hobby": ["sleep", "eat", "dreaming"]
}
```

Ȼ�󣬰��� `kyuubi` ���﷨��ʽ�������������ַ���ģ�壺

```
Hello! I am {{ name }}, and I am {{ age + " years old" }}.
My hobbies are:
{% for h in hobby %}
* {{ h }}{% if loop.isLastItem %}.{% else %};{% endif %}
{% endfor %}
```

ֻ��Ҫ��д���´��룬�Ϳ�������滻��

```cpp
Kyuubi kb = L"Hello! I am {{ name }}, and I am {{ age + \" years old\" }}.\r\n"
	L"My hobbies are:\r\n"
	L"{% for h in hobby %}\r\n"
	L"* {{ h }}{% if loop.isLastItem %}.{% else %};{% endif %}\r\n"
	L"{% endfor %}"_cs;
JsonObject data = JsonObject::object({
	{L"name", L"Tecyle"},
	{L"age", 18},
	{L"hobby", JsonObject::list({L"sleep", L"eat", L"dreaming"})}
	});
ConstString result;
EXPECT_TRUE(kb.genreateToString(&data, result));
ConstString expectedResult = L"Hello! I am Tecyle, and I am 18 years old.\r\n"
	L"My hobbies are:\r\n"
	L"* sleep;\r\n"
	L"* eat;\r\n"
	L"* dreaming.\r\n"_cs;
EXPECT_CONST_STRING_EQ(result, expectedResult);
```

���գ�������ַ���Ϊ��

```
Hello! I am Tecyle, and I am 18 years old.
My hobbies are:
* sleep;
* eat;
* dreaming.
```

## �ļ�ģ��

`kyuubi` ͬ��֧�ִ���չ��Ϊ `.wanling` ���ļ��ж���ģ���ַ��������滻���ݣ�����������ָ���ļ��С�

���� VS ��Ϊ `.wanling` �ļ�д���﷨�������������������Ч�����£�

![demo](images/wanling-demo.png)

## ���ɽ���Դ��

���˿�������ʱ����ģ���滻֮�⣬�����Ը���Ԥ��׼���õ�ģ�壬���ɶ�Ӧ�Ľ���Դ�롣

��Щҵ�񳡾��£���Ҫ�滻��ģ���ǹ̶��ģ�û��Ҫ������ʱ��ȥ����ģ�岢������
���ʱ�򣬿����� `KyuubiGenerator` Ԥ�Ƚ�ģ�����������ָ�Ȼ��
ֻ��Ҫ��������Ŀ�� include �Զ����ɵ�Դ���������뼴�ɡ�

## `kyuubi` �﷨���

### ��ֵ���ʽ

�� `{{` �� `}}` ��Χ�ı��ʽ����������ֵ���ʽ��
�������ǣ��Ӵ���� json �����м�����ʽ��ֵ�����滻��ԭλ���ϡ�

���磬����� json �����д�����Ϊ `name` �ĳ�Ա��ֵΪ `Tecyle`��
��ô��` I am {{ name }}` �ͻ�ʶ�� `name`���������滻Ϊ `Tecyle`��
�����滻��ԭ�ַ����У���Ϊ `I am Tecyle`��

��ֵ���ʽ��֧��������������ţ�����֧���� `a['b']` �� `a.b` �����ַ�ʽ�����ӳ�Ա��

��ֵ���ʽ�����ù� `|` ������ʹ�ù������������������Ͼ��ǶԱ��ʽ������н�һ������ĺ������á�

���磬�����±��ʽ `{{ name | camelCase }}`��
�京��Ϊ���� `name` Ӧ����Ϊ `camelCase` �Ĺ�������
�����������������Ϊ�������ʽ���ת��Ϊ�շ���ʽ��

�����`name` ������Ϊ `this-is-test`����ô��Ӧ�ù�������
����ͻ��Ϊ `thisIsTest`��

����������Ƕ�׵��ã����� `{{ name | filter1 | filter2(arg1, arg2) | ... }}`��

### raw ���

`{% raw %}` �� `{% endraw %}` ֮�䣬��ʾ�ڲ������ݲ����κν�����ԭ�������

### �����ṹ

�����ṹ�﷨���£�

```
{% if [condition] %}
    ...
{% elseif [condition] %}
    ...
{% else %}
    ...
{% endif %}
```

### ѭ���ṹ

ѭ���ṹ�﷨���£�

```
{% for (var, index) in vars %}
    ....
{% endfor %}
```

����Ƕ� map ���б�����������д��

```
(% for (key, value, index) in map %}
```

ѭ�������У�����ͨ�� `loop` ������ȡѭ����״̬��
Ŀǰ����֧�� `loop.isLastItem` ���ж�ѭ���Ƿ����һ�

### set ���

```
{% set var = expression %}
```

### filter ���

`filter` ����Զ�һ���ı�Ӧ��һ����������

```
{% filter name(params) %}
    ...

{% endfilter %}
```

## ע��

`{# ... #}` ��ʾע�͡�ע�ͻᱻ���ԡ�

## �հ״���

��ֵ���ʽ��ǰ��հ׶���ԭ��������

�����ע��ǰ��Ŀհ׶���ɾ����
���һ�����ռ��һ�У�����һ���ж����ᱣ����

�������ݻ��ڲ���������...