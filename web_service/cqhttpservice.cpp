#include <QSslConfiguration>
#include "cqhttpservice.h"
#include "myjson.h"

CqhttpService::CqhttpService(QObject *parent) : QObject(parent)
{
    initWS();

    connect(sig, SIGNAL(hostChanged(QString)), this, SLOT(openHost(QString)));
}

void CqhttpService::initWS()
{
    socket = new QWebSocket();

    connect(socket, SIGNAL(textMessageReceived(const QString&)), this, SLOT(messageReceived(const QString&)));

    connect(socket, &QWebSocket::connected, this, [=]{
        loopStarted();
        emit sig->socketStateChanged(true);
    });

    connect(socket, &QWebSocket::disconnected, this, [=]{
        emit sig->socketStateChanged(false);
    });
}

/// 连接socket时初始化一些数据
void CqhttpService::loopStarted()
{
    // 获取登录号信息
    {
        MyJson json;
        json.insert("action", "get_login_info");
        json.insert("echo", "get_login_info");
        socket->sendTextMessage(json.toBa());
    }
    // 获取好友列表
    {
        MyJson json;
        json.insert("action", "get_friend_list");
        json.insert("echo", "get_friend_list");
        socket->sendTextMessage(json.toBa());
    }

    // 获取群列表
    {
        MyJson json;
        json.insert("action", "get_group_list");
        json.insert("echo", "get_group_list");
        socket->sendTextMessage(json.toBa());
    }
}

void CqhttpService::openHost(QString host)
{
    // 设置安全套接字连接模式（不知道有啥用）
    QSslConfiguration config = socket->sslConfiguration();
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    config.setProtocol(QSsl::TlsV1SslV3);
    socket->setSslConfiguration(config);

    socket->open(host);
}

/// 收到消息的根入口
/// 字段说明：https://docs.go-cqhttp.org/event
void CqhttpService::messageReceived(const QString &message)
{
    MyJson json(message.toUtf8());

    // 先判断是不是主动发消息过去，回复的
    if (json.contains("echo"))
    {
        parseEchoMessage(json);
        return ;
    }

    JS(json, post_type);
    if (post_type == "meta_event") // 心跳，忽略
    {
    }
    else if (post_type == "message") // 消息
    {
        qDebug() << json;
        JS(json, message_type);
        if (message_type == "private") // 私信
        {
            parsePrivateMessage(json);
        }
        else if (message_type == "group") // 群消息
        {
            parseGroupMessage(json);
        }
        else
        {
            qDebug() << "未处理类型的消息：" << json;
        }
    }
    else if (post_type == "notice")
    {
        JS(json, notice_type);
        if (notice_type == "group_upload")
        {
            parseGroupUpload(json);
        }
        else
        {
            qDebug() << "未处理类型的通知：" << json;
        }
    }
    else
    {
        qDebug() << "未处理类型的数据" << json;
    }
}

void CqhttpService::parseEchoMessage(const MyJson &json)
{
    if (json.i("retcode") != 0)
    {
        qWarning() << "返回错误：" << json;
        return ;
    }
    JS(json, echo);
    if (echo == "get_login_info")
    {
        JL(json, user_id);
        JS(json, nickname);
        myId = user_id;
        myNickname = nickname;
    }
    else if (echo == "get_friend_list")
    {
        json.each("data", [=](MyJson fri) {
            JS(fri, nickname);
            JS(fri, remark); // 备注，如果为空则默认为nickname
            JL(fri, user_id);
            friendHash.insert(user_id, remark.isEmpty() ? nickname : remark);
        });
    }
    else if (echo == "get_group_list")
    {
        json.each("data", [=](MyJson group) {
            JL(group, group_id);
            JS(group, group_name);
            groupHash.insert(group_id, group_name);
        });
    }
    else
    {
        qDebug() << "未处理类型的返回：" << json;
    }
    return ;
}

void CqhttpService::parsePrivateMessage(const MyJson &json)
{
    JS(json, sub_type); // 好友：friend，群临时会话：group，群里自己发送：group_self
    JS(json, message); // 消息内容
    JS(json, raw_message);
    JL(json, message_id);

    JO(json, sender); // 发送者，但不保证存在
    JL(sender, user_id); // 发送者用户QQ号
    JS(sender, nickname);

    emit signalMessage(MsgBean(user_id, nickname, message, message_id, sub_type));
    qDebug() << "收到好友消息：" << user_id << nickname << message << message_id;

    // 图片消息：文字1\r\n[CQ:image,file=8f84df65ee005b52f7f798697765a81b.image,url=http://c2cpicdw.qpic.cn/offpic_new/1600631528//1600631528-3839913603-8F84DF65EE005B52F7F798697765A81B/0?term=3]\r\n文字二……
}

void CqhttpService::parseGroupMessage(const MyJson &json)
{
    JS(json, sub_type); // 正常：normal，匿名：anonymous，系统：notice
    JS(json, message);
    JS(json, raw_message);
    JL(json, group_id); // 群号
    JL(json, message_id);

    JO(json, sender); // 发送者，但不保证存在
    JL(sender, user_id); // 发送者QQ号
    JS(sender, nickname);
    JS(sender, card); // 群名片/备注，可能为空
    JS(sender, role); // 角色：owner/admin/member

    if (sub_type == "anonymous" && !json.value("anonymous").isNull()) // 是匿名消息
    {
        JO(json, anonymous);
        JL(anonymous, id); // 匿名用户ID
        JS(anonymous, name); // 匿名用户名称
        JS(anonymous, flag); // 匿名用户flag，在调用禁言API时需要传入
    }

    emit signalMessage(MsgBean(user_id, nickname, message, message_id, sub_type).group(group_id, groupHash.value(group_id), card));
    qDebug() << "收到群消息：" << group_id << groupHash.value(group_id) << user_id << friendHash.value(user_id) << message << message_id;
}

void CqhttpService::parseGroupUpload(const MyJson &json)
{
    JL(json, group_id); // QQ群号
    JL(json, user_id); // 发送者QQ号

    JO(json, file);
    JS(file, id); // 文件ID
    JS(file, name); // 文件名
    JL(file, size); // 文件大小（字节数）

    emit signalMessage(MsgBean(user_id, friendHash.value(user_id))
                       .group(group_id, groupHash.value(group_id))
                       .file(id, name, size));
    qDebug() << "收到群文件消息：" << group_id << groupHash.value(group_id) << user_id << friendHash.value(user_id) << name << size << id;
}
