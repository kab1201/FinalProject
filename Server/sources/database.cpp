#include "database.h"

DataBase::DataBase(QObject *parent)
    : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QPSQL", "myConnection");
    m_db.setHostName("localhost");
    m_db.setPort(5432);
    m_db.setDatabaseName("chat_db");
    m_db.setUserName("postgres");
    m_db.setPassword("bdfqyj7psql");
}

DataBase::~DataBase() {}

bool DataBase::openDatabase()
{
    if(m_db.open()) {
        QStringList qstrs;
        qstrs.append("SET NAMES 'utf8mb4'");
        qstrs.append("SET CHARACTER SET 'utf8mb4'");
        qstrs.append("SET SESSION collation_connection" \
                     " = 'utf8mb4_unicode_ci'");
        return true;
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText(m_db.lastError().text());
        msgBox.setInformativeText("Error");
        msgBox.exec();
        return false;
    }
}

bool DataBase::createTables()
{
    QSqlQuery query(m_db);
    bool result {0};
    QString str;
    str = "CREATE TABLE IF NOT EXISTS Users("
          "id SERIAL4 NOT NULL,"
          "name VARCHAR(255) NOT NULL,"
          "login VARCHAR(255) NOT NULL UNIQUE,"
          "email VARCHAR(255),"
          "regdate TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
          "last_login_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
          "CONSTRAINT pk_users PRIMARY KEY(id))";
    result = query.exec(str);
    if(!result)
    {
        qDebug() << "DataBase: error of create Users";
        qDebug() << query.lastError().text();
        return false;
    }

    str = "CREATE TABLE IF NOT EXISTS Hash("
          "id INT NOT NULL UNIQUE,"
          "hash INT NULL CHECK (hash > 0),"
          "salt VARCHAR(10) NULL,"
          "CONSTRAINT fk_hash_id FOREIGN KEY (id) REFERENCES Users(id) ON DELETE CASCADE ON UPDATE CASCADE)";
    result = query.exec(str);
    if(!result)
    {
        qDebug() << "DataBase: error of create Hash";
        qDebug() << query.lastError().text();
        return false;
    };

    query.prepare("SELECT 'True' FROM Users LIMIT 1");
    query.exec();
    if (query.size() == 0) {
        query.exec("CREATE OR REPLACE FUNCTION Trig_InsertNewRowToHash() "
                    "RETURNS TRIGGER AS "
                    "$function$ "
                    "BEGIN "
                        "INSERT INTO hash ( id ) VALUES ( NEW.id); "
                        "RETURN NEW; "
                    "END; "
                    "$function$ LANGUAGE plpgsql;");

        query.exec("CREATE TRIGGER NewUser_Insert_to_Hash AFTER INSERT ON Users FOR EACH ROW EXECUTE PROCEDURE Trig_InsertNewRowToHash();");

        str = "INSERT INTO Users(name, login, email)"
              "VALUES "
              "('all', 'all', 'all@mail.ru'),"
              "('TEST1', 'test1', 'test1@mail.ru'),"
              "('TEST2', 'test2', 'test2@mail.ru'),"
              "('TEST3', 'test3', 'test3@mail.ru'),"
              "('TEST4', 'test4', 'test4@mail.ru'),"
              "('TEST5', 'test5', 'test5@mail.ru')";
        result = query.exec(str);
        if(!result)
        {
            qDebug() << "DataBase: error of insert data to table Users";
            qDebug() << query.lastError().text();
            return false;
        };

        query.exec("UPDATE Hash "
                 "SET hash = 1583067709,"
                 "salt = 'Sqncgj3OFx'"
                 "WHERE id = 1");

        query.exec("UPDATE Hash "
                 "SET hash = 3430300287,"
                 "salt = 'vL@g`A6Gxp' "
                 "WHERE id = 2");

        query.exec("UPDATE Hash "
                 "SET hash = 2451056565,"
                 "salt = 'Zms(2<JT/>' "
                 "WHERE id = 3");

        query.exec("UPDATE Hash "
                 "SET hash = 3353989826,"
                 "salt = '-WC@4H!,[8' "
                 "WHERE id = 4");

        query.exec("UPDATE Hash "
                 "SET hash = 2870157325,"
                 "salt = 'gI~>.u7`X>' "
                 "WHERE id = 5");

        query.exec("UPDATE Hash "
                 "SET hash = 1958071822,"
                 "salt = 'b_T~W1T:R~'"
                 "WHERE id = 6");
    }

    str = "CREATE TABLE IF NOT EXISTS Messages("
          "id SERIAL4 NOT NULL,"
          "sender_id INT NOT NULL,"
          "recipient_id INT NOT NULL DEFAULT 1,"
          "message TEXT NOT NULL,"
          "creation_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
          "CONSTRAINT pk_messages PRIMARY KEY(id),"
          "CONSTRAINT fk_sender_id FOREIGN KEY (sender_id) REFERENCES Users (id) ON UPDATE CASCADE,"
          "CONSTRAINT fk_recipient_id FOREIGN KEY (recipient_id) REFERENCES Users(id) ON UPDATE CASCADE)";
    result = query.exec(str);
    if(!result)
    {
        qDebug() << "DataBase: error of create Messages";
        qDebug() << query.lastError().text();
        return false;
    };

    query.prepare("SELECT 'True' FROM Messages LIMIT 1");
    query.exec();
    if (query.size() == 0)
        str = "INSERT INTO Messages(sender_id, recipient_id, message)"
              "VALUES "
              "(2, 5, 'Hi, TEST4! How are you?'),"
              "(2, 3, 'I invite you to a meeting at 4:00 p.m.'),"
              "(3, 2, 'Hi, TEST1. Ok!'),"
              "(3, 1, 'Who lost their keys?'),"
              "(3, 1, 'Good day, all!'),"
              "(3, 5, 'Good day, TEST4!')";
        result = query.exec(str);
        if(!result)
        {
            qDebug() << "DataBase: error of insert data to table Messages";
            qDebug() << query.lastError().text();
            return false;
        };

    str = "CREATE OR REPLACE VIEW vw_messages AS "
          "SELECT "
          "m.id, "
          "s.login AS sender, "
          "r.login AS recipient, "
          "m.message, "
          "m.creation_date, "
          "m.sender_id, "
          "m.recipient_id "
          "FROM "
          "messages AS m "
          "JOIN users AS s ON "
          "m.sender_id = s.id "
          "JOIN users AS r ON "
          "m.recipient_id = r.id";
    result = query.exec(str);
    if(!result)
    {
        qDebug() << "DataBase: error of create view vw_messages";
        qDebug() << query.lastError().text();
        return false;
    };

    return true;
}

QString DataBase::getUserById(QString id)
{
    QSqlQuery query(m_db);
    QString str = "SELECT login FROM users WHERE id=" + id + "";
    if(query.exec(str)) {
        if(query.next())
            return query.value(0).toString();
    }
    return "";
}

QSqlDatabase DataBase::getDb() const
{
    return m_db;
}

int DataBase::getUserIdByLogin(QString login)
{
    QSqlQuery query(m_db);
    QString str = "SELECT id FROM users WHERE login='" + login + "'";
    if(query.exec(str)) {
        if(query.next())
            return query.value(0).toInt();
    }
    return -1;
}

int DataBase::getUserIdByName(QString name)
{
    QSqlQuery query(m_db);
    QString str = "SELECT id FROM users WHERE name='" + name + "'";
    if(query.exec(str)) {
        if(query.next())
            return query.value(0).toInt();
    }
    return -1;
}

QVector<QString> DataBase::getAllUsers()
{
    //qDebug() << "DB is open: " << (m_db.open());
    QSqlQuery query(m_db);
    QVector<QString> users {};
    QString str = "SELECT login FROM users WHERE (id != 1) ORDER BY login";
    bool result = query.exec(str);
    if(result) {
        while(query.next()) {
            users.push_back(query.value(0).toString());
        }
    }
    return users;
}

int DataBase::addUser(QString name, QString login, QString password)
{

    int res = checkNewLogin(login);
    /*
    switch(res + 1) {
    case 3:
        qInfo() << "User with login '" << login << "' not added in database! The login must contain letters and numbers only!";
        break;
    case 4:
        qInfo() << "Login 'all' is reserved!" ;
        break;
    case 2:
        qInfo() << "User with login '" << login << "' is already exists!" ;
        break;
    default:
        break;
    }
    */
    if (res !=0) {
        return res + 1;
    }

    res = checkNewPassword(password);
    /*
    switch(res + 4) {
    case 5:
        qInfo() << "User with login '" << login << "' not added in database! Password must be equal to or more than 5 characters long!";
        break;
    case 6:
        qInfo() << "User with login '" << login << "' not added in database! Password must not exceed 20 characters long!";
        break;
    case 7:
        qInfo() << "User with login '" << login << "' not added in database! Password contains invalid characters!" ;
        break;
    default:
        break;
    }
    */
    if (res !=0) {
        return res + 4;
    }

    try {
        int idNewUser = -1;

        QSqlQuery query(m_db);
        query.prepare("INSERT INTO users(name, login) "
                      "VALUES(:name, :login);");
        query.bindValue(":name", name);
        query.bindValue(":login", login);
        if(!query.exec()) {
            throw QString("User with login " + login + " not added in database! " + m_db.lastError().text());
        }
        else {
            idNewUser = query.lastInsertId().toInt();
        }
        string salt = createSalt();
        uint hash = createHashPassword(password.toStdString() + salt);
        query.prepare("UPDATE Hash SET hash = :hash, salt = :salt WHERE id = :id");
        query.bindValue(":hash", QString::number(hash).toUInt());
        query.bindValue(":salt", QString::fromStdString(salt));
        query.bindValue(":id", idNewUser);
        if(!query.exec()) {
            throw QString("User with login " + login + " not added in database! " + m_db.lastError().text());
        }
    }
    catch (const QString& error_message)
    {
        qInfo() << error_message;
        return 0;
    }
    return 1;
}

int DataBase::checkUserByLoginAndPassword(QString login, QString password)
{
    auto userID = getUserIdByLogin(login);
    if (userID == -1)
        return 1;  //Invalid login!

    string salt = getSalt(userID);
    if (getHashPassword(userID) != createHashPassword(password.toStdString() + salt))
        return 2; //Invalid password!

    setLoginTime(login);
    return 0;
}

bool DataBase::isAlphaNumeric(const string &str)
{
    auto it = find_if_not(str.begin(), str.end(), [](char const &c) {
        if (c>=0 and c<=255) {
            return isalnum(c);
        }
        else {
            return 0;
        }
    });
    return it == str.end();
}

int DataBase::checkNewLogin(const QString &login)
{
    if (!isAlphaNumeric(login.toStdString()))
        return 2; //The login must contain letters and numbers only

    if (login == "all")
        return 3; //This login is reserved

    auto userID = getUserIdByLogin(login);
    if (userID > 0)
        return 1; //This login already exists

    return 0;
}

int DataBase::checkNewPassword(const QString& psw)
{
    if (psw.length() < minPswLen)
        return 1; //Password must be equal to or more than 5 characters long

    if (psw.length() > maxPswLen)
        return 2; //Password must not exceed 20 characters long

    const char* pattern = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                          "abcdefghijklmnopqrstuvwxyz"
                          "0123456789"
                          "`~!@#$%^&*()_-+=|/[]{}:;',.<>\?\ ";

    if (psw.toStdString().find_first_not_of(pattern) != string::npos)
        return 3; //Password contains invalid characters

    return 0;
}

int DataBase::getHashPassword(const int userID) const
{
    QSqlQuery query(m_db);
    QString str = "SELECT hash FROM hash WHERE (id = " +  QString::number(userID) + ")";
    if(query.exec(str)) {
        if(query.next())
            return query.value(0).toInt();
    }
    return -1;
}

string DataBase::getSalt(const int userID) const
{
    string res;
    QSqlQuery query(m_db);
    QString str = "SELECT salt FROM hash WHERE (id = " +  QString::number(userID) + ")";
    if(query.exec(str)) {
        if(query.next())
            res = query.value(0).toString().toStdString();
    }
    return res;
}

uint DataBase::createHashPassword(const string &psw)
{
    // Convert string to char*[]
    size_t lenPsw = psw.length();
    char* userPsw = new char[lenPsw];
    memcpy(userPsw, psw.c_str(), lenPsw);

    // Hash password (including salt)
    uint* userHashPsw = sha1(userPsw, lenPsw);

    // Clear
    delete[] userPsw;

    return *userHashPsw;
}

string DataBase::createSalt()
{
    // Using the computer’s internal clock to generate the seed.
    const auto p = std::chrono::system_clock::now() - std::chrono::hours(24);
    auto seed = p.time_since_epoch().count();
    // Quickly generating pseudo random integers between 1 and 2147483646
    default_random_engine randomEngine(seed);
    // Converting those random integers into the range [33, 126] such that they’re uniformly distributed.
    uniform_int_distribution<int> distribution(0x21, 0x7E);
    auto randomReadableCharacter = [&]() { return distribution(randomEngine); };

    size_t size = 10;
    string salt;
    generate_n(back_inserter(salt), size, randomReadableCharacter);

    return salt;
}

bool DataBase::addMessageToAll(QString sender, QString text)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO messages(sender_id, message) "
                  "VALUES(:sender_id, :message);");
    query.bindValue(":sender_id", getUserIdByLogin(sender));
    query.bindValue(":message", text);
    if(query.exec()) {
        //qInfo() << "Message to all from " << sender << " added in database!" ;
        return true;
    }
    else {
        //qInfo() << "Message to all from " << sender << " not added in database!" ;
        return false;
    }
}

bool DataBase::addPrivateMessage(QString sender, QString recipient, QString text)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO messages(sender_id, recipient_id, message) "
                  "VALUES(:sender_id, :recipient_id, :message);");
    query.bindValue(":sender_id", getUserIdByLogin(sender));
    query.bindValue(":recipient_id", getUserIdByLogin(recipient));
    query.bindValue(":message", text);
    if(query.exec()) {
        //qInfo() << "Message to " <<  recipient << "from " << sender << " added in database!" ;
        return true;
    }
    else {
        //qInfo() << "Message to " <<  recipient << "from " << sender << " not added in database!" ;
        return false;
    }
}

QVector<QString> DataBase::getMessagesBetweenTwoUsers(QString user1, QString user2, QString placeOfUsed)
{
    int userID_1 = getUserIdByLogin(user1);
    int userID_2 = getUserIdByLogin(user2);
    QString userId1 = QString::number(userID_1);
    QString userId2 = QString::number(userID_2);
    QVector<QString> privateMessages {};
    QString message {};
    QSqlQuery query(m_db);
    QString str = "SELECT sender, recipient, message, creation_date FROM vw_messages ";
    if(userID_1 != 1 and userID_2 != 1) {
        str = str +
        "WHERE (sender_id=" + userId1 + " AND recipient_id=" + userId2 + ") "
        "OR (sender_id=" + userId2 + " AND recipient_id=" + userId1 + ") "
        "ORDER BY creation_date";
    }
    else if(userID_1 != 1 and userID_2 == 1) {
        if (placeOfUsed == "Client") {
            str = str +
            "WHERE ((sender_id=" + userId1 + ") OR (recipient_id=" + userId1 + ")) OR (recipient_id = 1) "
            "ORDER BY creation_date";
        }
        else {
            str = str +
            "WHERE (sender_id=" + userId1 + ") "
            "ORDER BY creation_date";
        }
    }
    else if(userID_1 == 1 and userID_2 != 1) {
        str = str +
        "WHERE (recipient_id=" + userId2 + ") "
        "ORDER BY creation_date";
    }
    else {
        str = "SELECT t.* FROM (" + str +
        "ORDER BY creation_date DESC "
        "LIMIT 20) AS t "
        "ORDER BY t.creation_date";
    }

    if(query.exec(str)) {
        while(query.next()) {
            message = query.value(0).toString() + ";" + query.value(1).toString() + ";" + query.value(2).toString() + ";";
            privateMessages.push_back(message);
        }
    }
    return privateMessages;
}

int DataBase::setBan(const QString &login, bool value)
{
    QSqlQuery query(m_db);
    QString str = "UPDATE users SET isBaned = :isBaned WHERE (login = :login);";
    query.prepare(str);
    query.bindValue(":isBaned", value);
    query.bindValue(":login", login);
    bool result = query.exec();
    if(!result) {
        qInfo() << query.lastError().text();
        return -1;
    }
    return 0;
}

int DataBase::setLoginTime(const QString &login)
{
    QSqlQuery query(m_db);
    QString str = "UPDATE users SET last_login_date = CURRENT_TIMESTAMP WHERE (login = :login);";
    query.prepare(str);
    query.bindValue(":login", login);
    bool result = query.exec();
    if(!result) {
        qInfo() << query.lastError().text();
        return -1;
    }
    return 0;
}
