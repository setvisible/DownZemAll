/* - DownZemAll! - Copyright (C) 2019-present Sebastien Vavassori
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DIALOGS_URL_FORM_WIDGET_H
#define DIALOGS_URL_FORM_WIDGET_H

#include <QtWidgets/QWidget>

class ResourceItem;

class QLabel;
class QLineEdit;

namespace Ui {
class UrlFormWidget;
}

class UrlFormWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UrlFormWidget(QWidget *parent = Q_NULLPTR);
    ~UrlFormWidget() Q_DECL_OVERRIDE;

    void setExternalUrlLabelAndLineEdit(QLabel *urlLabel, QLineEdit *urlLineEdit);
    void setReferringPage(const QString &referringPage);
    void hideCustomFile();

    bool isValid() const;

    ResourceItem* createResourceItem() const;
    void setResource(const ResourceItem *resource);

    QString url() const;

    bool isChildrenEnabled() const;
    void setChildrenEnabled(bool enabled);

    QString currentPath() const;
    void setCurrentPath(const QString &path);

    QStringList pathHistory() const;
    void setPathHistory(const QStringList &paths);

    QString currentMask() const;
    void setCurrentMask(const QString &text);

signals:
    void changed(QString);

private:
    Ui::UrlFormWidget *ui;
    void recalculateSize();
};

#endif // DIALOGS_URL_FORM_WIDGET_H
