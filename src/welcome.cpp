#include "welcome.h"
#include "login.h"
#include <QTextBrowser>
#include <QHBoxLayout>
// 补充布局头文件（代码本身不动，注释说明）
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QList>

/**
 * @brief Welcome构造函数：欢迎界面窗体构造
 * @param parent 父控件指针，Qt控件树管理
 */
Welcome::Welcome(QWidget *parent)
    : QMainWindow(parent)
    , m_loginWindow(nullptr) { // 登录窗口指针初始化为空，防止野指针
    initUserInterface();        // 调用自定义UI初始化函数，统一构建页面
}

/**
 * @brief 欢迎窗口析构函数，释放堆开辟的登录窗口内存，避免内存泄漏
 */
Welcome::~Welcome() {
    delete m_loginWindow;
}

/**
 * @brief initUserInterface：界面初始化核心函数，完成全部控件创建、布局、样式、信号绑定
 */
void Welcome::initUserInterface() {
    setWindowTitle("校园二手书交易管理系统"); // 设置窗口标题文字
    setFixedSize(1000, 900);          // 布局不变，固定窗体宽高，禁止拖动缩放窗口
    setStyleSheet("background-color: #fafafa;");  // 浅灰背景，设置整个窗口底色

    QWidget *centralWidget = new QWidget(this);    // QMainWindow必须配置中心部件，所有布局依托中心控件
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget); // 全局垂直总布局，从上到下排布所有控件
    mainLayout->setContentsMargins(40, 30, 40, 30); // 设置布局四边留白距离
    mainLayout->setSpacing(20);                    // 设置布局内部控件之间的间距

    // 主标题（黑色）：系统大标题标签
    QLabel *titleLabel = new QLabel("校园二手书交易管理系统", this);
    titleLabel->setAlignment(Qt::AlignCenter); // 标题文字居中对齐
    titleLabel->setStyleSheet(
        "font-size: 36px;"
        "font-weight: 700;"
        "color: #1a1a1a;"          // 黑色
        "letter-spacing: 2px;"
        "margin-bottom: 5px;"
        );

    // 副标题（灰色）：预留副标题位置
    QLabel *subtitleLabel = new QLabel(" ", this);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet(
        "font-size: 18px;"
        "color: #666666;"           // 中灰色
        "margin-bottom: 15px;"
        );

    // 功能说明窗口（黑白边框）：功能卡片外框分组容器
    QGroupBox *infoBox = new QGroupBox("系统功能一览", this);
    infoBox->setStyleSheet(
        "QGroupBox {"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   color: #1a1a1a;"
        "   border: 2px solid #cccccc;"
        "   border-radius: 12px;"
        "   margin-top: 10px;"
        "   padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 20px;"
        "   padding: 0 8px;"
        "   background-color: #fafafa;"
        "}"
        );

    QGridLayout *infoLayout = new QGridLayout(infoBox); // 网格布局，实现功能卡片两列排版
    infoLayout->setSpacing(12);                        // 卡片之间间距
    infoLayout->setContentsMargins(15, 15, 15, 15);    // 分组内部边距

    // 自定义结构体：封装单个功能卡片所需图标、标题、描述三项数据
    struct FuncItem {
        QString icon;
        QString title;
        QString desc;
    };
    // 系统六大功能数据集合，对应项目全部设计功能
    QList<FuncItem> items = {
        {"🔍", "图书浏览与查询", "支持按书名、作者、ISBN、价格区间、卖家学号、交易状态多条件筛选"},
        {"📕", "发布图书", "卖家录入信息，自动生成唯一编号，状态默认为“在售”"},
        {"🛒", "购买图书", "支持编号或书名查找，二次确认后记录买家和交易时间"},
        {"📤", "我的图书", "下架自己发布的在售图书，需身份验证"},
        {"📊", "统计报表", "全局统计 + 按时间段统计销售额、分类成交量"},
        {"⚙️", "管理员功能", "删除图书、清空数据、查看所有交易记录（仅管理员）"}
    };

    int row = 0, col = 0; // 网格行列下标，用于两列自动换行
    // 循环遍历功能数组，动态批量生成每一张功能卡片
    for (const auto &item : items) {
        QGroupBox *funcCard = new QGroupBox();
        funcCard->setFixedHeight(150);   // 布局不变，单张卡片固定高度
        funcCard->setStyleSheet(
            "QGroupBox {"
            "   border: 1px solid #dddddd;"
            "   border-radius: 8px;"
            "   background-color: #ffffff;"
            "   margin-top: 5px;"
            "}"
            "QGroupBox::title { subcontrol-origin: margin; left: 10px; }"
            );
        QVBoxLayout *cardLayout = new QVBoxLayout(funcCard); // 卡片内部垂直布局：图标+标题+描述
        cardLayout->setSpacing(5);
        cardLayout->setContentsMargins(8, 8, 8, 8);

        QLabel *iconLabel = new QLabel(item.icon);    // 功能图标标签
        iconLabel->setStyleSheet("font-size: 28px;");
        iconLabel->setAlignment(Qt::AlignCenter);
        QLabel *titleLabel2 = new QLabel(item.title); // 功能名称标签
        titleLabel2->setStyleSheet("font-size: 14px; font-weight: bold; color: #1a1a1a;");
        titleLabel2->setAlignment(Qt::AlignCenter);
        QLabel *descLabel = new QLabel(item.desc);    // 功能说明标签
        descLabel->setWordWrap(true);                 // 开启文字自动换行
        descLabel->setAlignment(Qt::AlignCenter);
        descLabel->setStyleSheet("font-size: 10px; color: #555555;");

        cardLayout->addWidget(iconLabel);    // 图标装入卡片内部布局
        cardLayout->addWidget(titleLabel2); // 标题装入卡片内部布局
        cardLayout->addWidget(descLabel);   // 描述装入卡片内部布局
        infoLayout->addWidget(funcCard, row, col); // 卡片添加进外层网格布局
        col++;                                     // 列下标自增
        if (col == 2) { col = 0; row++; }          // 列满2列，重置列、行+1实现换行
    }

    // 开发者信息（灰色）：页面底部开发信息标签
    QLabel *devInfoLabel = new QLabel("开发者：蔡裕  学号：202505487015", this);
    devInfoLabel->setAlignment(Qt::AlignCenter);
    devInfoLabel->setStyleSheet(
        "font-size: 12px;"
        "color: #888888;"
        "margin-top: 10px;"
        );

    // 进入系统按钮（黑色圆角）：页面跳转触发按钮
    QPushButton *enterButton = new QPushButton("进入系统", this);
    enterButton->setFixedSize(240, 50);
    enterButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #1a1a1a;"
        "   color: #ffffff;"
        "   font-size: 18px;"
        "   font-weight: 600;"
        "   border-radius: 25px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #333333; }"
        "QPushButton:pressed { background-color: #000000; }"
        );
    // Qt信号槽：按钮点击信号绑定跳转槽函数
    connect(enterButton, &QPushButton::clicked, this, &Welcome::onEnterButtonClicked);

    // 全部控件依次装入顶层主垂直布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(infoBox);
    mainLayout->addStretch(); // 弹性空白占位，把按钮挤压居中
    mainLayout->addWidget(enterButton, 0, Qt::AlignCenter);
    mainLayout->addWidget(devInfoLabel);
}

/**
 * @brief onEnterButtonClicked：进入系统按钮点击槽函数，实现欢迎页→登录页跳转
 */
void Welcome::onEnterButtonClicked() {
    this->hide();               // 隐藏当前欢迎窗口
    m_loginWindow = new Login();// 动态创建登录界面对象
    m_loginWindow->show();      // 展示登录窗口
}