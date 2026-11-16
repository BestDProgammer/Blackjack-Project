#include "ShoeController.h"
#include "SideBetEV.h"
#include <QtWidgets>
#include <array>
#include <algorithm>
#include <cmath>
#include <vector>

namespace {
const QStringList ranks{"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
const QStringList suits{QString::fromUtf8("♣"), QString::fromUtf8("♦"), QString::fromUtf8("♥"), QString::fromUtf8("♠")};
const QStringList suitNames{"Clubs", "Diamonds", "Hearts", "Spades"};
const QStringList outcomes{"Flush", "Straight", "Three of a kind", "Straight flush", "Suited trips"};
QString percent(double v, int decimals = 3) { return QString(v >= 0 ? "+" : "") + QString::number(v * 100, 'f', decimals) + "%"; }
QLabel* label(const QString& text, const char* name = "", QWidget* parent = nullptr) {
    auto* w = new QLabel(text, parent); w->setObjectName(name); return w;
}
QPushButton* button(const QString& text, const char* name = "") {
    auto* w = new QPushButton(text); w->setObjectName(name); w->setProperty("primary", QString(name) == "primary"); w->setCursor(Qt::PointingHandCursor); return w;
}
QFrame* panel() { auto* p = new QFrame; p->setObjectName("panel"); return p; }

class CardFan : public QWidget {
public:
    CardFan() { setMinimumHeight(235); }
    void paintEvent(QPaintEvent*) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        for (int i = 0; i < 3; ++i) {
            p.save(); p.translate(width() / 2 + (i - 1) * 60, height() / 2 + (i == 1 ? -10 : 12)); p.rotate((i - 1) * 17);
            p.setPen(Qt::NoPen); p.setBrush(QColor(0, 0, 0, 45)); p.drawRoundedRect(QRectF(-54, -76, 112, 160), 9, 9);
            p.setBrush(QColor("#efe9d8")); p.drawRoundedRect(QRectF(-56, -80, 112, 160), 9, 9);
            p.setPen(QColor(i == 1 ? "#a34049" : "#234133"));
            p.setFont(QFont("Georgia", 19)); p.drawText(QRectF(-45, -72, 44, 32), Qt::AlignLeft, QStringList{"A", "K", "Q"}[i]);
            p.setFont(QFont("Georgia", 17)); p.drawText(QRectF(-45, -44, 34, 30), Qt::AlignLeft, suits[i == 1 ? 2 : 3]);
            p.setFont(QFont("Georgia", 47)); p.drawText(QRectF(-46, -35, 92, 87), Qt::AlignCenter, suits[i == 1 ? 2 : 3]);
            p.restore();
        }
    }
};

class Trend : public QWidget {
public:
    std::vector<double> values;
    Trend() { setMinimumHeight(50); setMaximumHeight(60); }
    void paintEvent(QPaintEvent*) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        QRectF area = rect().adjusted(2, 10, -2, -10);
        p.setPen(QPen(QColor("#29443c"), 1, Qt::DashLine));
        p.drawLine(area.left(), area.center().y(), area.right(), area.center().y());
        if (values.empty()) return;
        auto [lo, hi] = std::minmax_element(values.begin(), values.end());
        double span = std::max(*hi - *lo, 0.0001), low = (*lo + *hi - span) / 2;
        QPainterPath line;
        for (size_t i = 0; i < values.size(); ++i) {
            QPointF pt(area.left() + area.width() * i / std::max(size_t(1), values.size() - 1),
                       area.bottom() - (values[i] - low) / span * area.height());
            if (i == 0) line.moveTo(pt); else line.lineTo(pt);
        }
        if (values.size() == 1) line.lineTo(area.right(), line.currentPosition().y());
        auto fill = line; fill.lineTo(area.bottomRight()); fill.lineTo(area.bottomLeft()); fill.closeSubpath();
        QLinearGradient gradient(area.topLeft(), area.bottomLeft());
        gradient.setColorAt(0, QColor(196, 166, 98, 45)); gradient.setColorAt(1, QColor(196, 166, 98, 0));
        p.fillPath(fill, gradient); p.setPen(QPen(QColor("#cbb278"), 2)); p.drawPath(line);
    }
};

class TableApp : public QWidget {
    ShoeController controller;
    QStackedWidget* pages;
    QSpinBox* deckInput;
    QComboBox* paytable;
    std::array<QDoubleSpinBox*, 5> payoutInputs{};
    QCheckBox* setupAuto;
    QCheckBox* autoMode;
    QCheckBox* enable21;
    std::array<QPushButton*, 52> cards{};
    std::array<QLabel*, 4> suitTotals{};
    std::array<QLabel*, 5> probabilities{};
    std::array<QLabel*, 5> payoutLabels{};
    QLabel *remaining, *removed, *decksLeft, *sessionInfo, *ev, *evStatus, *evDetail, *delta, *recent, *winRate;
    QPushButton *undo, *updateButton;
    QProgressBar* penetration;
    Trend* trend;
    PayoutStructure21Plus3 payouts;
    std::array<double, 5> selectedPayouts{};
    std::array<int, 52> calculatedCounts{};
    double baseline = 0;
    bool calculated = false;
    bool active21 = true;

public:
    TableApp() {
        setWindowTitle("Felt — Blackjack shoe laboratory"); resize(1440, 960); setMinimumSize(1120, 800);
        setStyleSheet(R"(
            QWidget { background: #101b18; color: #e9ece5; font-family: 'Avenir Next', 'Segoe UI', sans-serif; font-size: 13px; }
            QLabel { background: transparent; }
            QFrame#sidebar { background: #0b1412; border-right: 1px solid #26362f; }
            QLabel#brand { color: #dec58c; font-family: Georgia; font-size: 35px; font-weight: bold; }
            QLabel#eyebrow { color: #bca46d; font-size: 11px; font-weight: 600; letter-spacing: 2px; }
            QLabel#title { font-family: Georgia; font-size: 34px; color: #f2eee3; }
            QLabel#muted { color: #90a399; }
            QLabel#section { font-size: 17px; font-weight: 600; }
            QLabel#stat { font-size: 32px; font-weight: 500; color: #f1ecdf; }
            QLabel#ev { font-size: 45px; font-weight: 500; }
            QLabel#gold { color: #d1b77b; }
            QFrame#panel { background: #14251e; border: 1px solid #2c4035; border-radius: 12px; }
            QFrame#hero { background: #1b3529; border: 1px solid #496043; border-radius: 16px; }
            QPushButton { background: #1d3027; border: 1px solid #3a4d40; border-radius: 7px; padding: 10px 16px; font-weight: 600; }
            QPushButton:hover { background: #2a4234; border-color: #bba471; }
            QPushButton:pressed { background: #38513d; }
            QPushButton:disabled { color: #65756b; border-color: #2b3a30; background: #19261f; }
            QPushButton[primary="true"] { background: #d2b67a; color: #15231a; border-color: #d2b67a; }
            QPushButton[primary="true"]:hover { background: #e6cd96; }
            QPushButton#nav { text-align: left; border: none; color: #91a497; background: transparent; padding: 13px 8px; }
            QPushButton#nav:checked { background: #1d3026; color: #dbc28c; border-left: 2px solid #cfb578; }
            QPushButton#card { background: #eeeade; color: #26372e; border: 1px solid #b6b7a4; border-radius: 6px; font-family: Georgia; font-size: 20px; padding: 6px 2px; }
            QPushButton#card[red="true"] { color: #a23d46; }
            QPushButton#card:hover { background: #fff9e9; border: 2px solid #d2b67a; }
            QPushButton#card:pressed { background: #d1b77b; }
            QPushButton#card:disabled { background: #213328; color: #5b6e60; border: 1px dashed #4a5a49; }
            QSpinBox, QDoubleSpinBox, QComboBox { background: #0e1c16; color: #eeeade; border: 1px solid #3a4f40; border-radius: 6px; padding: 9px; min-height: 20px; }
            QComboBox QAbstractItemView { background: #14251e; selection-background-color: #38513d; }
            QCheckBox { background: transparent; spacing: 10px; padding: 6px 0; }
            QCheckBox::indicator { width: 18px; height: 18px; border-radius: 4px; border: 1px solid #718369; background: #101e16; }
            QCheckBox::indicator:checked { background: #cbb278; image: none; border: 4px solid #43533a; }
            QProgressBar { background: #0c1912; border: none; border-radius: 3px; max-height: 6px; }
            QProgressBar::chunk { background: #cbb278; border-radius: 3px; }
            QScrollArea { border: none; background: transparent; }
            QToolTip { background: #eeeade; color: #14251e; border: none; }
        )");
        auto* root = new QHBoxLayout(this); root->setContentsMargins(0, 0, 0, 0); root->setSpacing(0);
        auto* sidebar = new QFrame; sidebar->setObjectName("sidebar"); sidebar->setFixedWidth(206);
        auto* nav = new QVBoxLayout(sidebar); nav->setContentsMargins(24, 30, 24, 24); nav->setSpacing(10);
        nav->addWidget(label("♠ felt", "brand")); nav->addWidget(label("THE SHOE LABORATORY", "eyebrow")); nav->addSpacing(42);
        auto* setupNav = button("01   Table setup", "nav"); auto* trackerNav = button("02   Shoe tracker", "nav");
        setupNav->setCheckable(true); trackerNav->setCheckable(true); setupNav->setChecked(true);
        trackerNav->setEnabled(false); nav->addWidget(setupNav); nav->addWidget(trackerNav); nav->addStretch();
        nav->addWidget(label("MANUAL INPUT", "eyebrow"));
        auto* note = label("Every card counts.\nExact composition.\nExact probabilities.", "muted"); note->setStyleSheet("line-height: 150%;"); nav->addWidget(note);
        nav->addSpacing(14); nav->addWidget(label("BLACKJACK  /  v0.1", "muted")); root->addWidget(sidebar);
        pages = new QStackedWidget;
        // Preserve usable card targets and readable results on smaller displays.
        pages->setMinimumSize(1150, 940);
        auto* scroll = new QScrollArea;
        scroll->setWidgetResizable(true);
        scroll->setWidget(pages);
        root->addWidget(scroll, 1);
        pages->addWidget(buildSetup()); pages->addWidget(buildTracker());
        connect(setupNav, &QPushButton::clicked, this, [this] { openSetup(); });
        connect(trackerNav, &QPushButton::clicked, this, [this] { pages->setCurrentIndex(1); });
        connect(pages, &QStackedWidget::currentChanged, this, [=](int i) { setupNav->setChecked(i == 0); trackerNav->setChecked(i == 1); });
        connect(findChild<QPushButton*>("start"), &QPushButton::clicked, this, [this, trackerNav] { startSession(); trackerNav->setEnabled(true); });
    }

private:
    QWidget* buildSetup() {
        auto* page = new QWidget; auto* layout = new QVBoxLayout(page); layout->setContentsMargins(42, 34, 42, 30); layout->setSpacing(20);
        layout->addWidget(label("YOUR TABLE. YOUR PARAMETERS.", "eyebrow")); layout->addWidget(label("Set the table.", "title"));
        layout->addWidget(label("Build a shoe, choose your paytable, and explore what every card changes.", "muted"));
        auto* body = new QHBoxLayout; body->setSpacing(26); layout->addLayout(body, 1);
        auto* form = panel(); auto* f = new QVBoxLayout(form); f->setContentsMargins(26, 24, 26, 24); f->setSpacing(8);
        f->addWidget(label("01   Shoe configuration", "section")); f->addWidget(label("Number of decks", "muted"));
        deckInput = new QSpinBox; deckInput->setRange(1, 16); deckInput->setValue(6); deckInput->setSuffix(" decks"); deckInput->setObjectName("decksInput"); f->addWidget(deckInput);
        f->addSpacing(6); f->addWidget(label("02   Side bets & payouts", "section"));
        enable21 = new QCheckBox("21 + 3   ·   Three-card poker"); enable21->setChecked(true); f->addWidget(enable21);
        auto* coming = new QCheckBox("Perfect Pairs   ·   Coming later"); coming->setEnabled(false); f->addWidget(coming);
        paytable = new QComboBox; paytable->addItems({"Classic · 9:1 on every winning hand", "Tiered · 5 / 10 / 30 / 40 / 100", "Custom paytable"}); f->addWidget(paytable);
        auto* payoutGrid = new QGridLayout; payoutGrid->setHorizontalSpacing(12);
        for (int i = 0; i < 5; ++i) {
            payoutGrid->addWidget(label(outcomes[i], "muted"), i, 0);
            payoutInputs[i] = new QDoubleSpinBox; payoutInputs[i]->setStyleSheet("padding: 4px 9px;"); payoutInputs[i]->setRange(0, 100000); payoutInputs[i]->setDecimals(2); payoutInputs[i]->setValue(9); payoutInputs[i]->setSuffix(" : 1"); payoutInputs[i]->setEnabled(false);
            payoutGrid->addWidget(payoutInputs[i], i, 1);
        }
        f->addLayout(payoutGrid); f->addWidget(label("Net winnings per unit wagered; losing hands return −1.", "muted"));
        connect(paytable, &QComboBox::currentIndexChanged, this, [this](int index) {
            std::array<double, 5> tiered{5, 10, 30, 40, 100};
            for (int i = 0; i < 5; ++i) { payoutInputs[i]->setEnabled(index == 2); if (index < 2) payoutInputs[i]->setValue(index == 0 ? 9 : tiered[i]); }
        });
        f->addSpacing(8); f->addWidget(label("03   Calculation mode", "section"));
        setupAuto = new QCheckBox("Automatically update EV after each card"); setupAuto->setChecked(true); f->addWidget(setupAuto); f->addStretch();
        body->addWidget(form, 3);
        auto* hero = new QFrame; hero->setObjectName("hero"); auto* h = new QVBoxLayout(hero); h->setContentsMargins(30, 36, 30, 30); h->setSpacing(20);
        h->addWidget(label("COMPOSITION IS EVERYTHING", "eyebrow"));
        h->addWidget(new CardFan);
        auto* heroTitle = label("A clearer view\nof the next hand.", "title"); heroTitle->setWordWrap(true); h->addWidget(heroTitle);
        auto* desc = label("Remove exposed cards. Watch the probabilities shift. Understand the shoe before the next round.", "muted"); desc->setWordWrap(true); h->addWidget(desc);
        h->addStretch(); h->addWidget(label("52 CARD CATEGORIES  ·  EXACT ENUMERATION", "eyebrow")); body->addWidget(hero, 2);
        auto* foot = new QHBoxLayout; foot->addWidget(label("A fresh shoe starts when you enter the tracker.", "muted")); foot->addStretch();
        auto* start = button("Start tracking   →", "primary"); start->setObjectName("start"); start->setStyleSheet("background: #d2b67a; color: #15231a; padding: 14px 28px;"); foot->addWidget(start); layout->addLayout(foot);
        return page;
    }

    QWidget* buildTracker() {
        auto* page = new QWidget; auto* outer = new QVBoxLayout(page); outer->setContentsMargins(34, 30, 34, 24); outer->setSpacing(20);
        auto* heading = new QHBoxLayout; auto* titles = new QVBoxLayout;
        titles->addWidget(label("LIVE SHOE / MANUAL ENTRY", "eyebrow")); titles->addWidget(label("Read the shoe.", "title"));
        sessionInfo = label("", "muted"); titles->addWidget(sessionInfo); heading->addLayout(titles); heading->addStretch();
        auto* settings = button("Table settings"); heading->addWidget(settings); connect(settings, &QPushButton::clicked, this, [this] { openSetup(); });
        auto* shuffle = button("↻  Shuffle shoe", "primary"); shuffle->setObjectName("shuffle"); heading->addWidget(shuffle);
        connect(shuffle, &QPushButton::clicked, this, [this] { controller.shuffle(); trend->values.clear(); calculated = false; refresh(); recalculate(); }); outer->addLayout(heading);
        auto* stats = new QHBoxLayout; stats->setSpacing(14);
        for (int i = 0; i < 3; ++i) {
            auto* box = panel(); auto* b = new QVBoxLayout(box); b->setContentsMargins(20, 16, 20, 16);
            b->addWidget(label(QStringList{"CARDS REMAINING", "CARDS REMOVED", "DECKS REMAINING"}[i], "eyebrow"));
            auto* number = label("0", "stat"); b->addWidget(number); if (i == 0) remaining = number; if (i == 1) removed = number; if (i == 2) decksLeft = number; stats->addWidget(box);
        }
        outer->addLayout(stats);
        auto* body = new QHBoxLayout; body->setSpacing(20); outer->addLayout(body, 1);
        auto* gridPanel = panel(); auto* g = new QVBoxLayout(gridPanel); g->setContentsMargins(20, 22, 20, 20); g->setSpacing(8);
        g->addWidget(label("Remove exposed cards", "section")); g->addWidget(label("Click a card to remove one copy. The number below is the remaining count.", "muted"));
        auto* grid = new QGridLayout; grid->setHorizontalSpacing(6); grid->setVerticalSpacing(10);
        for (int suit = 0; suit < 4; ++suit) {
            auto* suitHeader = new QHBoxLayout; auto* suitLabel = label(suits[suit] + "  " + suitNames[suit]);
            suitLabel->setStyleSheet(suit == 1 || suit == 2 ? "color: #db8e8b; font-weight: 600;" : "color: #c3d1bf; font-weight: 600;");
            suitHeader->addWidget(suitLabel); suitHeader->addStretch(); suitTotals[suit] = label("", "muted"); suitHeader->addWidget(suitTotals[suit]); grid->addLayout(suitHeader, suit * 2, 0, 1, 13);
            for (int rank = 0; rank < 13; ++rank) {
                auto* card = button("", "card"); card->setProperty("red", suit == 1 || suit == 2); card->setMinimumSize(39, 64); card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                card->setAccessibleName(ranks[rank] + " of " + suitNames[suit]); cards[rank * 4 + suit] = card;
                grid->addWidget(card, suit * 2 + 1, rank);
                connect(card, &QPushButton::clicked, this, [this, rank, suit] { if (controller.removeCard(rank, suit)) changed(); });
            }
        }
        g->addLayout(grid, 1); g->addSpacing(6);
        auto* undoRow = new QHBoxLayout; undo = button("↶  Undo last card"); undo->setObjectName("undo"); undo->setShortcut(QKeySequence::Undo); undoRow->addWidget(undo); undoRow->addStretch(); undoRow->addWidget(label("⌘ / Ctrl + Z to undo", "muted")); g->addLayout(undoRow);
        connect(undo, &QPushButton::clicked, this, [this] { if (controller.undoLastRemoval()) changed(); });
        g->addWidget(label("RECENT REMOVALS", "eyebrow")); recent = label("No cards removed yet.", "muted"); recent->setWordWrap(true); g->addWidget(recent);
        penetration = new QProgressBar; penetration->setTextVisible(false); g->addWidget(penetration);
        body->addWidget(gridPanel, 7);
        auto* right = panel(); right->setMinimumWidth(290); right->setMaximumWidth(390); auto* r = new QVBoxLayout(right); r->setContentsMargins(22, 22, 22, 22); r->setSpacing(7);
        auto* evHeading = new QHBoxLayout; evHeading->addWidget(label("21 + 3", "section")); evHeading->addStretch(); evStatus = label("CURRENT", "gold"); evHeading->addWidget(evStatus); r->addLayout(evHeading);
        r->addWidget(label("EXPECTED NET RETURN", "eyebrow")); ev = label("—", "ev"); r->addWidget(ev);
        evDetail = label("", "muted"); evDetail->setWordWrap(true); r->addWidget(evDetail);
        delta = label("", "gold"); r->addWidget(delta); trend = new Trend; r->addWidget(trend);
        r->addWidget(label("SESSION EV TREND · LAST 100 UPDATES", "muted"));
        autoMode = new QCheckBox("Automatic EV updates"); autoMode->setChecked(true); autoMode->setObjectName("autoMode"); r->addWidget(autoMode);
        updateButton = button("Update EV", "primary"); updateButton->setObjectName("updateEV"); r->addWidget(updateButton);
        connect(updateButton, &QPushButton::clicked, this, [this] { recalculate(); });
        connect(autoMode, &QCheckBox::toggled, this, [this](bool automatic) { if (automatic) recalculate(); else refreshStatus(); });
        auto* line = new QFrame; line->setFrameShape(QFrame::HLine); r->addWidget(line);
        r->addWidget(label("OUTCOME PROBABILITIES", "eyebrow"));
        auto* table = new QGridLayout; table->setVerticalSpacing(6); table->addWidget(label("Hand", "muted"), 0, 0); table->addWidget(label("Pays", "muted"), 0, 1); table->addWidget(label("Chance", "muted"), 0, 2);
        for (int i = 0; i < 5; ++i) { table->addWidget(label(outcomes[i]), i + 1, 0); payoutLabels[i] = label("", "muted"); table->addWidget(payoutLabels[i], i + 1, 1); probabilities[i] = label("—"); probabilities[i]->setAlignment(Qt::AlignRight); table->addWidget(probabilities[i], i + 1, 2); }
        r->addLayout(table); winRate = label("", "muted"); r->addWidget(winRate); r->addStretch();
        auto* explanation = label("Calculated from the remaining shoe, before the next round is dealt.", "muted"); explanation->setWordWrap(true); r->addWidget(explanation); body->addWidget(right, 3);
        auto* footer = new QHBoxLayout; footer->addWidget(label("●  Manual card entry", "gold")); footer->addStretch(); footer->addWidget(label("Exact probabilities. No simulation.", "muted")); outer->addLayout(footer);
        return page;
    }

    void openSetup() {
        setupAuto->setChecked(autoMode->isChecked());
        if (pages->currentIndex() == 1) findChild<QPushButton*>("start")->setText("Start a fresh session   →");
        pages->setCurrentIndex(0);
    }
    void startSession() {
        controller.configure(deckInput->value());
        active21 = enable21->isChecked();
        for (int i = 0; i < 5; ++i) { selectedPayouts[i] = payoutInputs[i]->value(); payoutLabels[i]->setText(QString::number(selectedPayouts[i], 'g', 6) + ":1"); }
        payouts.setPayouts(selectedPayouts[0], selectedPayouts[1], selectedPayouts[2], selectedPayouts[3], selectedPayouts[4]);
        { QSignalBlocker blocker(autoMode); autoMode->setChecked(setupAuto->isChecked()); }
        sessionInfo->setText(QString("%1 decks  /  %2  /  Exact shoe composition").arg(controller.getDecks()).arg(enable21->isChecked() ? "21 + 3" : "No side bets selected"));
        trend->values.clear(); calculated = false;
        if (active21) baseline = calculateEV21Plus3(controller.getShoe(), payouts);
        refresh(); recalculate(); pages->setCurrentIndex(1);
    }
    void changed() { refresh(); if (autoMode->isChecked()) recalculate(); else refreshStatus(); }
    void refresh() {
        const auto& shoe = controller.getShoe();
        remaining->setText(QString::number(shoe.getRemaining())); removed->setText(QString::number(controller.history().size()));
        decksLeft->setText(QString::number(shoe.getRemaining() / 52.0, 'f', 2));
        penetration->setRange(0, controller.getDecks() * 52); penetration->setValue(int(controller.history().size()));
        undo->setEnabled(!controller.history().empty());
        for (int suit = 0; suit < 4; ++suit) {
            int total = 0;
            for (int rank = 0; rank < 13; ++rank) {
                int count = shoe.getCount(rank, suit); total += count; auto* card = cards[rank * 4 + suit];
                card->setText(ranks[rank] + suits[suit] + "\n" + QString::number(count)); card->setEnabled(count > 0);
                card->setToolTip(QString("Remove %1 of %2 · %3 copies left").arg(ranks[rank], suitNames[suit]).arg(count));
            }
            suitTotals[suit]->setText(QString::number(total) + " cards");
        }
        QStringList recentCards;
        const auto& history = controller.history();
        for (size_t i = history.size(); i > 0 && i + 10 > history.size(); --i) { auto [rank, suit] = history[i - 1]; recentCards << ranks[rank] + suits[suit]; }
        recent->setText(recentCards.isEmpty() ? "No cards removed yet." : recentCards.join("    "));
        refreshStatus();
    }
    bool isCurrent() const {
        if (!calculated) return false;
        for (int i = 0; i < 52; ++i) if (calculatedCounts[i] != controller.getShoe().getCardCount(i)) return false;
        return true;
    }
    void refreshStatus() {
        bool enabled = active21, enough = controller.getShoe().getRemaining() >= 3;
        updateButton->setEnabled(enabled && enough); autoMode->setEnabled(enabled);
        evStatus->setText(!enabled ? "DISABLED" : !enough ? "UNAVAILABLE" : isCurrent() ? "CURRENT" : "OUTDATED");
        evStatus->setStyleSheet(isCurrent() && enough ? "color: #95c6a2;" : "color: #dbb477;");
        if (!enough && enabled) { calculated = false; ev->setText("—"); evDetail->setText("At least 3 cards are needed to calculate 21 + 3."); delta->clear(); for (auto* p : probabilities) p->setText("—"); winRate->clear(); }
        else if (enabled && !isCurrent()) evDetail->setText(calculated ? "Shoe changed. Press Update EV to refresh the displayed results." : "Press Update EV to calculate for the current shoe.");
        else if (enabled && isCurrent()) evDetail->setText(QString("Per unit wagered · calculated at %1 cards").arg(controller.getShoe().getRemaining()));
    }
    void recalculate() {
        if (!active21) {
            ev->setText("—"); evDetail->setText("21 + 3 is disabled. Enable it in Table settings."); delta->clear(); winRate->clear();
            for (auto* p : probabilities) p->setText("—"); refreshStatus(); return;
        }
        if (controller.getShoe().getRemaining() < 3) { calculated = false; refreshStatus(); return; }
        auto p = calculateProbabilities21Plus3(controller.getShoe());
        std::array<double, 5> probs{p.flush, p.straight, p.threeOfAKind, p.straightFlush, p.suitedTrips};
        double value = -p.loss; for (int i = 0; i < 5; ++i) { value += probs[i] * selectedPayouts[i]; probabilities[i]->setText(QString::number(probs[i] * 100, 'f', 3) + "%"); }
        bool changedComposition = !isCurrent();
        for (int i = 0; i < 52; ++i) calculatedCounts[i] = controller.getShoe().getCardCount(i);
        calculated = true; ev->setText(percent(value)); ev->setStyleSheet(value >= 0 ? "color: #9ccea9;" : "color: #e2aaa1;");
        double difference = std::abs(value - baseline) < 0.00000001 ? 0 : (value - baseline) * 100;
        delta->setText(QString(difference >= 0 ? "+" : "") + QString::number(difference, 'f', 3) + " pp from fresh shoe"); winRate->setText("Any winning hand: " + QString::number((1 - p.loss) * 100, 'f', 3) + "%");
        if (changedComposition) { trend->values.push_back(value); if (trend->values.size() > 100) trend->values.erase(trend->values.begin()); trend->update(); }
        refreshStatus();
    }
};
}

int main(int argc, char** argv) {
    QApplication app(argc, argv); app.setApplicationName("Felt");
    TableApp window; window.show();
    if (app.arguments().contains("--smoke-test")) {
        QTimer::singleShot(100, &window, [&] {
            window.grab().save("/tmp/felt-setup.png");
            auto click = [&](const char* name) { window.findChild<QPushButton*>(name)->click(); };
            auto require = [](bool ok) { if (!ok) qFatal("UI smoke test failed"); };
            click("start"); QApplication::processEvents();
            window.grab().save("/tmp/felt-tracker-initial.png");
            auto allCards = window.findChildren<QPushButton*>("card"); require(allCards.size() == 52);
            auto* card = allCards.first(); require(card->text().endsWith("6")); card->click(); require(card->text().endsWith("5"));
            click("undo"); require(card->text().endsWith("6"));
            window.findChild<QCheckBox*>("autoMode")->setChecked(false); card->click();
            bool outdated = false; for (auto* l : window.findChildren<QLabel*>()) if (l->text() == "OUTDATED") outdated = true; require(outdated);
            click("updateEV");
            for (int i = 0; i < 5; ++i) card->click(); require(!card->isEnabled());
            click("shuffle"); require(card->isEnabled() && card->text().endsWith("6"));
            QApplication::processEvents(); window.grab().save("/tmp/felt-tracker.png");
            // Verify the minimum-shoe state and restoration, using a single deck.
            for (auto* b : window.findChildren<QPushButton*>()) if (b->text() == "Table settings") { b->click(); break; }
            window.findChild<QSpinBox*>("decksInput")->setValue(1); click("start");
            for (int i = 0; i < 50; ++i) allCards[i]->click();
            bool unavailable = false;
            for (auto* l : window.findChildren<QLabel*>()) if (l->text() == "UNAVAILABLE") unavailable = true;
            require(unavailable && !window.findChild<QPushButton*>("updateEV")->isEnabled());
            click("undo"); require(window.findChild<QPushButton*>("updateEV")->isEnabled()); click("updateEV");
            qInfo("UI smoke test passed; screenshots: /tmp/felt-setup.png and /tmp/felt-tracker.png"); app.quit();
        });
    }
    return app.exec();
}
