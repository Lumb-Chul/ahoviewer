#ifndef _PAGE_H_
#define _PAGE_H_

#include <atomic>
#include <gtkmm.h>

#include "curler.h"
#include "site.h"
#include "imagefetcher.h"
#include "imagelist.h"

namespace AhoViewer
{
    namespace Booru
    {
        class Image;
        class Page : public Gtk::ScrolledWindow,
                     public ImageList::Widget
        {
        public:
            typedef sigc::signal<void, Page*> SignalClosedType;
            typedef sigc::signal<void, const std::string> SignalDownloadErrorType;
            typedef sigc::signal<void, size_t, size_t> SignalSaveProgressType;

            Page(Gtk::Menu *menu);
            virtual ~Page();

            virtual void set_selected(const size_t index);

            void search(std::shared_ptr<Site> site, const std::string &tags);
            void save_image(const std::string &path, const std::shared_ptr<Image> &img);
            void save_images(const std::string &path);
            bool ask_cancel_save();

            ImageFetcher& get_image_fetcher() const { return *m_ImageFetcher; }
            Gtk::Widget* get_tab() const { return m_Tab; }
            std::shared_ptr<Site> get_site() const { return m_Site; }
            std::shared_ptr<ImageList> get_imagelist() const { return m_ImageList; }
            std::string get_tags() const { return m_Tags; }
            size_t get_page_num() const { return m_Page; }
            bool is_saving() const { return m_Saving; }

            SignalClosedType signal_closed() const { return m_SignalClosed; }
            SignalDownloadErrorType signal_no_results() const { return m_SignalDownloadError; }
            SignalSaveProgressType signal_save_progress() const { return m_SignalSaveProgress; }
        private:
            void cancel_save();
            void get_posts();
            bool get_next_page();
            void on_posts_downloaded();
            void on_selection_changed();
            void on_value_changed();
            bool on_button_press_event(GdkEventButton *e);

            Gtk::Menu *m_PopupMenu;
            std::unique_ptr<ImageFetcher> m_ImageFetcher;
            Gtk::IconView *m_IconView;
            Gtk::HBox *m_Tab;
            Gtk::Image *m_TabIcon;
            Gtk::Label *m_TabLabel;
            Gtk::Button *m_TabButton;

            std::shared_ptr<ImageList> m_ImageList;
            std::shared_ptr<Site> m_Site;
            Curler m_Curler, m_CountsCurler;

            std::string m_Tags, m_Path;
            size_t m_Page, m_NumPosts,
                   m_PostsCount,
                   m_SaveImagesTotal;
            std::atomic<size_t> m_SaveImagesCurrent;
            bool m_LastPage, m_Saving;
            std::shared_ptr<xmlDocument> m_Posts;

            Glib::RefPtr<Gio::Cancellable> m_SaveCancel;
            Glib::Threads::Thread *m_GetPostsThread,
                                  *m_SaveImagesThread;
            Glib::Dispatcher m_SignalPostsDownloaded,
                             m_SignalSaveProgressDisp;

            sigc::connection m_GetNextPageConn;

            SignalClosedType m_SignalClosed;
            SignalDownloadErrorType m_SignalDownloadError;
            SignalSaveProgressType m_SignalSaveProgress;
        };
    }
}

#endif /* _PAGE_H_ */
