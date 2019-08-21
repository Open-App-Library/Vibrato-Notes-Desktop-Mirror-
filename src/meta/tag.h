#ifndef TAG_H
#define TAG_H
#include <QObject>
#include <QString>
#include <QUuid>
#include <QDateTime>

class SQLManager;

#define TAG_DEFAULT_TITLE "Untitled Tag"

class Tag : public QObject
{
  Q_OBJECT
public:
  Tag(SQLManager *sql_manager,
      QUuid       uuid          = QUuid::createUuid(),
      QString     title         = TAG_DEFAULT_TITLE,
      QDateTime   date_created  = QDateTime::currentDateTime(),
      QDateTime   date_modified = QDateTime::currentDateTime(),
      int         row           = -255,
      bool        encrypted     = false);

  QUuid uuid() const;
  void setUuid(QUuid uuid);

  QString title() const;
  void setTitle(const QString value);

  QDateTime dateCreated() const;
  void setDateCreated(QDateTime dateCreated, bool emitChangeSignal=true);

  QDateTime dateModified() const;
  void setDateModified(QDateTime dateModified, bool emitChangeSignal=true);

  int row() const;
  void setRow(int row);

  bool encrypted() const;
  void setEncrypred(bool encrypted);

signals:
  // General change function - whenever any property is changed
  void changed(Tag *tag);

  // Property signals
  void uuidChanged(Tag *tag);
  void titleChanged(Tag *tag);
  void dateModifiedChanged(Tag *tag);
  void rowChanged(Tag *tag);
  void encryptedChanged(Tag *tag);

private slots:
  void handleChange(Tag *tag);

private:
  SQLManager *m_sql_manager;
  QUuid       m_uuid;
  QString     m_title;
  QDateTime   m_date_created;
  QDateTime   m_date_modified;
  int         m_row;
  bool        m_encrypted;

};

#endif // TAG_H
