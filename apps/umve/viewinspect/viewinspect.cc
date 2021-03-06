#include <iostream>
#include <limits>

#include "mve/view.h"
#include "mve/image.h"
#include "mve/plyfile.h"
#include "mve/imageexif.h"

#include "guihelpers.h"
#include "scenemanager.h"
#include "viewinspect.h"

ViewInspect::ViewInspect (QWidget* parent)
    : QWidget(parent)
{
    this->scroll_image = new ScrollImage();
    this->embeddings = new QComboBox();
    this->embeddings->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    this->embeddings->setEditable(false);
    this->embeddings->setEnabled(false);

    this->label_name = new QLabel("");
    this->label_dimension = new QLabel("--");
    this->label_memory = new QLabel("--");

    /* Create GUIs, actions & menus. */
    this->create_detail_frame();
    this->create_actions();
    this->update_actions();
    this->create_menus();

    /* Connect signals. */
    this->connect(this->embeddings, SIGNAL(activated(QString const&)),
        this, SLOT(on_embedding_selected(QString const&)));
    this->connect(this->scroll_image->get_image(),
        SIGNAL(mouse_clicked(int, int, QMouseEvent*)),
        this, SLOT(on_image_clicked(int, int, QMouseEvent*)));

    this->connect(&SceneManager::get(), SIGNAL(scene_selected(mve::Scene::Ptr)),
        this, SLOT(on_scene_selected(mve::Scene::Ptr)));
    this->connect(&SceneManager::get(), SIGNAL(view_selected(mve::View::Ptr)),
        this, SLOT(on_view_selected(mve::View::Ptr)));

    /* Setup layout. */
    QHBoxLayout* head_layout = new QHBoxLayout();
    head_layout->addWidget(this->embeddings);
    head_layout->addSpacerItem(new QSpacerItem(10, 0));
    head_layout->addWidget(this->label_name);
    head_layout->addSpacerItem(new QSpacerItem(0, 0,
        QSizePolicy::Expanding, QSizePolicy::Minimum));
    head_layout->addWidget(new QLabel(tr("Dimension:")));
    head_layout->addWidget(this->label_dimension);
    head_layout->addSpacerItem(new QSpacerItem(10, 0));
    head_layout->addWidget(new QLabel(tr("Memory:")));
    head_layout->addWidget(this->label_memory);

    QVBoxLayout* image_layout = new QVBoxLayout();
    image_layout->addWidget(this->toolbar);
    image_layout->addLayout(head_layout);
    image_layout->addWidget(this->scroll_image, 1);

    QHBoxLayout* main_layout = new QHBoxLayout(this);
    main_layout->addLayout(image_layout, 1);
    main_layout->addWidget(this->image_details);
}

/* ---------------------------------------------------------------- */

void
ViewInspect::create_detail_frame (void)
{
    this->tfunc = new TransferFunctionWidget();
    this->inspector = new ImageInspectorWidget();
    this->operations = new ImageOperationsWidget();
    this->exif_viewer = new QTextEdit();
    this->exif_viewer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    this->populate_exif_viewer();

    this->connect(this->tfunc, SIGNAL(function_changed(TransferFunction)),
        this, SLOT(on_tf_changed(TransferFunction)));
    this->connect(this->operations, SIGNAL(signal_reload_embeddings()),
        this, SLOT(on_reload_embeddings()));
    this->connect(this->operations, SIGNAL(signal_select_embedding
        (QString const&)), this, SLOT(on_embedding_selected(QString const&)));

    this->image_details = new QTabWidget();//tr("Details"));
    this->image_details->setTabPosition(QTabWidget::East);
    this->image_details->addTab(this->operations, tr("Operations"));
    this->image_details->addTab(this->inspector, tr("Image Inspector"));
    this->image_details->addTab(this->tfunc, tr("Transfer Function"));
    this->image_details->addTab(this->exif_viewer, tr("EXIF"));
    this->image_details->hide();
}

/* ---------------------------------------------------------------- */

void
ViewInspect::create_actions (void)
{
    this->action_open = new QAction(QIcon(":/images/icon_open_file.svg"),
        tr("Open view/image"), this);
    this->connect(this->action_open, SIGNAL(triggered()),
        this, SLOT(on_open()));

    this->action_reload = new QAction(QIcon(":/images/icon_revert.svg"),
        tr("Reload file"), this);
    this->connect(this->action_reload, SIGNAL(triggered()),
        this, SLOT(on_view_reload()));

    this->action_save_view = new QAction(QIcon(":/images/icon_save.svg"),
        tr("Save view"), this);
    this->connect(this->action_save_view, SIGNAL(triggered()),
        this, SLOT(on_save_view()));

    this->action_export_ply = new QAction
        (QIcon(":/images/icon_export_ply.svg"),
        tr("Export Scanalize PLY"), this);
    this->connect(this->action_export_ply, SIGNAL(triggered()),
        this, SLOT(on_ply_export()));

    this->action_export_image = new QAction(QIcon
        (":/images/icon_screenshot.svg"), tr("Export Image"), this);
    this->connect(this->action_export_image, SIGNAL(triggered()),
        this, SLOT(on_image_export()));

    this->action_zoom_in = new QAction(QIcon(":/images/icon_zoom_in.svg"),
        tr("Zoom &In"), this);
    this->action_zoom_in->setShortcut(tr("Ctrl++"));
    //this->action_zoom_in->setEnabled(false);
    this->connect(this->action_zoom_in, SIGNAL(triggered()),
        this, SLOT(on_zoom_in()));

    this->action_zoom_out = new QAction(QIcon(":/images/icon_zoom_out.svg"),
        tr("Zoom &Out"), this);
    this->action_zoom_out->setShortcut(tr("Ctrl+-"));
    //this->action_zoom_out->setEnabled(false);
    this->connect(this->action_zoom_out, SIGNAL(triggered()),
        this, SLOT(on_zoom_out()));

    this->action_zoom_reset = new QAction(QIcon(":/images/icon_zoom_reset.svg"),
        tr("&Reset Size"), this);
    this->action_zoom_reset->setShortcut(tr("Ctrl+0"));
    this->connect(this->action_zoom_reset, SIGNAL(triggered()),
        this, SLOT(on_normal_size()));

    this->action_zoom_fit = new QAction(QIcon(":/images/icon_zoom_page.svg"),
        tr("&Fit to Window"), this);
    this->action_zoom_fit->setShortcut(tr("Ctrl+1"));
    this->action_zoom_fit->setCheckable(true);
    this->connect(this->action_zoom_fit, SIGNAL(triggered()),
        this, SLOT(on_fit_to_window()));

    this->action_show_details = new QAction
        (QIcon(":/images/icon_toolbox.svg"),
        tr("Show &Details"), this);
    this->action_show_details->setCheckable(true);
    this->connect(this->action_show_details, SIGNAL(triggered()),
        this, SLOT(on_details_toggled()));

    this->action_copy_embedding = new QAction
        (QIcon(":/images/icon_copy.svg"),
        tr("&Copy Embedding"), this);
    this->connect(this->action_copy_embedding, SIGNAL(triggered()),
        this, SLOT(on_copy_embedding()));

    this->action_del_embedding = new QAction
        (QIcon(":/images/icon_delete.svg"),
        tr("Delete Embedding"), this);
    this->connect(this->action_del_embedding, SIGNAL(triggered()),
        this, SLOT(on_del_embedding()));
}

/* ---------------------------------------------------------------- */

void
ViewInspect::create_menus (void)
{
    this->toolbar = new QToolBar("Viewer tools");
    this->toolbar->addAction(this->action_open);
    this->toolbar->addAction(this->action_reload);
    this->toolbar->addAction(this->action_save_view);
    this->toolbar->addAction(this->action_export_ply);
    this->toolbar->addAction(this->action_export_image);
    this->toolbar->addSeparator();
    this->toolbar->addAction(this->action_zoom_in);
    this->toolbar->addAction(this->action_zoom_out);
    this->toolbar->addAction(this->action_zoom_reset);
    this->toolbar->addAction(this->action_zoom_fit);
    this->toolbar->addSeparator();
    this->toolbar->addAction(this->action_copy_embedding);
    this->toolbar->addAction(this->action_del_embedding);
    //this->toolbar->addSeparator();
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->toolbar->addWidget(get_expander());
    this->toolbar->addAction(this->action_show_details);
}

/* ---------------------------------------------------------------- */

void
ViewInspect::show_details (bool show)
{
    this->image_details->setVisible(show);
}

/* ---------------------------------------------------------------- */

void
ViewInspect::update_actions (void)
{
    bool active = this->scroll_image->get_pixmap() != 0;
    bool fit = this->action_zoom_fit->isChecked();
    this->action_zoom_fit->setEnabled(active);
    this->action_zoom_in->setEnabled(active && !fit);
    this->action_zoom_out->setEnabled(active && !fit);
    this->action_zoom_reset->setEnabled(active && !fit);
}

/* ---------------------------------------------------------------- */

void
ViewInspect::load_file (QString filename)
{
    /* Check extension. Handle ".mve" files manually. */
    if (filename.rightRef(4) == ".mve")
        this->load_mve_file(filename);
    else if (filename.rightRef(4) == ".ply")
        this->load_ply_file(filename);
    else
        this->load_image_file(filename);
}

/* ---------------------------------------------------------------- */

void
ViewInspect::load_image_file (QString filename)
{
    QImage image(filename);
    if (image.isNull())
    {
        QMessageBox::warning(this, tr("Image Viewer"),
            tr("Cannot load image %1").arg(filename));
        return;
    }

    this->scroll_image->set_pixmap(QPixmap::fromImage(image));
    this->action_zoom_fit->setEnabled(true);
    this->update_actions();
}

/* ---------------------------------------------------------------- */

void
ViewInspect::load_mve_file (QString filename)
{
    mve::View::Ptr view(mve::View::create());
    try
    { view->load_mve_file(filename.toStdString()); }
    catch (std::exception& e)
    {
        QMessageBox::warning(this, tr("Image Viewer"),
            tr("Cannot load %1:\n%2").arg(filename, e.what()));
        return;
    }

    this->on_view_selected(view);
}

/* ---------------------------------------------------------------- */

void
ViewInspect::load_ply_file (QString filename)
{
    try
    {
        mve::FloatImage::Ptr img(mve::geom::load_ply_depthmap(filename.toStdString()));
        this->image = img;
        this->tfunc->set_color_assignment(img->channels());
        this->display_image(img);
        this->inspector->set_image(img);
    }
    catch (std::exception& e)
    {
        QMessageBox::warning(this, tr("Image Viewer"),
            tr("Cannot load %1:\n%2").arg(filename, e.what()));
        return;
    }

}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_view_selected (mve::View::Ptr view)
{
    this->reset();
    this->view = view;
    if (!this->view.get())
        return;

    this->load_recent_embedding();
    this->populate_embeddings();
    this->label_name->setText(view->get_name().c_str());
    this->populate_exif_viewer();
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_scene_selected (mve::Scene::Ptr /*scene*/)
{
    this->reset();
}

/* ---------------------------------------------------------------- */

void
ViewInspect::set_embedding (std::string const& name)
{
    //std::cout << "Loading view embedding \"" << name << "\"" << std::endl;
    if (!this->view.get())
    {
        QMessageBox::warning(this, tr("Image Viewer"),
            tr("No view loaded").arg(QString(name.c_str())));
        return;
    }

    /* Request the embedding. */
    mve::ImageBase::Ptr img(this->view->get_image(name));
    if (img.get() == 0)
    {
        QMessageBox::warning(this, tr("Image Viewer"),
            tr("Embedding not available: %1").arg(QString(name.c_str())));
        return;
    }

    this->recent_embedding = name;
    this->image = img;

    /* Update labels. */
    std::stringstream ss;
    ss << img->width() << "x" << img->height() << "x"
        << img->channels() << " (" << img->get_type_string()
        << ")";

    this->label_dimension->setText(tr("%1").arg(ss.str().c_str()));
    this->label_memory->setText(tr("%1 KB").arg(image->get_byte_size() / 1024));

    /* Display image. */
    switch (img->get_type())
    {
        case mve::IMAGE_TYPE_UINT8:
            this->tfunc->set_color_assignment(0); // Implement?
            this->tfunc->show_minmax_sliders(false);
            this->func = this->tfunc->get_function();
            this->inspector->set_transfer_function(this->func);
            this->display_image((mve::ByteImage::Ptr)img);
            break;

        case mve::IMAGE_TYPE_FLOAT:
        {
            mve::FloatImage::Ptr fimg(img);
            /* Set minimum and maximum value. */
            float min_value = std::numeric_limits<float>::max();
            float max_value = -std::numeric_limits<float>::max();
            for (int i = 0; i < fimg->get_value_amount(); ++i)
            {
                float value = fimg->at(i);
                min_value = std::min(min_value, value);
                max_value = std::max(max_value, value);
            }

            this->tfunc->set_minmax_range(min_value, max_value);
            this->tfunc->set_clamp_range(min_value, max_value);
            this->tfunc->set_color_assignment(fimg->channels());
            this->tfunc->show_minmax_sliders(true);
            this->func = this->tfunc->get_function();

            this->inspector->set_transfer_function(this->func);
            this->display_image(fimg);
            break;
        }

        default:
            QMessageBox::warning(this, tr("Image Viewer"),
                tr("Unsupported embedding: %1").arg(QString(name.c_str())));
            break;
    }

    /* Update image inspector. */
    this->inspector->set_image(this->image);
}

/* ---------------------------------------------------------------- */

void
ViewInspect::display_image (mve::ByteImage::Ptr img)
{
    std::size_t iw = img->width();
    std::size_t ih = img->height();
    std::size_t ic = img->channels();
    bool is_gray = (ic == 1 || ic == 2);
    bool has_alpha = (ic == 2 || ic == 4);

    //std::cout << "Populating pixmap..." << std::endl;

    QImage img_qimage(iw, ih, QImage::Format_ARGB32);
    {
        QPainter painter(&img_qimage);
        std::size_t inpos = 0;
        for (std::size_t y = 0; y < ih; ++y)
            for (std::size_t x = 0; x < iw; ++x)
            {
                unsigned char alpha = has_alpha
                    ? img->at(inpos + 1 + !is_gray * 2)
                    : 255;
                unsigned int argb
                    = (alpha << 24)
                    | (img->at(inpos) << 16)
                    | (img->at(inpos + !is_gray * 1) << 8)
                    | (img->at(inpos + !is_gray * 2) << 0);

                img_qimage.setPixel(x, y, argb);
                inpos += ic;
            }
    }

    this->scroll_image->set_pixmap(QPixmap::fromImage(img_qimage));
    this->action_zoom_fit->setEnabled(true);
    this->update_actions();
}

/* ---------------------------------------------------------------- */

void
ViewInspect::display_image (mve::FloatImage::Ptr img)
{
    std::size_t iw = img->width();
    std::size_t ih = img->height();
    std::size_t ic = img->channels();

    QImage img_qimage(iw, ih, QImage::Format_RGB32);
    {
        //QPainter painter(&img_qimage);
        std::size_t inpos = 0;
        for (std::size_t y = 0; y < ih; ++y)
            for (std::size_t x = 0; x < iw; ++x)
            {
                float red = img->at(inpos + this->func.red);
                float green = img->at(inpos + this->func.green);
                float blue = img->at(inpos + this->func.blue);

                if (this->func.highlight_values >= 0.0f
                    && red >= 0.0f && red <= this->func.highlight_values
                    && green >= 0.0f && green <= this->func.highlight_values
                    && blue >= 0.0f && blue <= this->func.highlight_values)
                {
                    red = 0.5f;
                    green = 0.0f;
                    blue = 0.5f;
                }
                else if (MATH_ISNAN(red) || MATH_ISNAN(green)
                    || MATH_ISNAN(blue) || MATH_ISINF(red)
                    || MATH_ISINF(green) || MATH_ISINF(blue))
                {
                    red = 1.0f;
                    green = 1.0f;
                    blue = 0.0f;
                }
                else
                {
                    red = this->func.evaluate(red);
                    green = this->func.evaluate(green);
                    blue = this->func.evaluate(blue);
                }

                unsigned int vr = (unsigned int)(red * 255.0f + 0.5f);
                unsigned int vg = (unsigned int)(green * 255.0f + 0.5f);
                unsigned int vb = (unsigned int)(blue * 255.0f + 0.5f);
                unsigned int rgb = vr << 16 | vg << 8 | vb;
                img_qimage.setPixel(x, y, rgb);
                inpos += ic;
            }
    }

    this->scroll_image->set_pixmap(QPixmap::fromImage(img_qimage));
    this->action_zoom_fit->setEnabled(true);
    this->update_actions();
}

/* ---------------------------------------------------------------- */

void
ViewInspect::load_recent_embedding (void)
{
    /* If no embedding is set, fall back to undistorted image. */
    if (this->recent_embedding.empty()
        || !this->view->has_embedding(this->recent_embedding))
        this->recent_embedding = "undistorted";

    /* Give up. */
    if (!this->view->has_embedding(this->recent_embedding))
        return;

    /* Load embedding. */
    this->set_embedding(this->recent_embedding);
}

/* ---------------------------------------------------------------- */

void
ViewInspect::populate_embeddings (void)
{
    this->embeddings->clear();
    if (!this->view.get())
        return;

    std::vector<std::string> vec;

    mve::View::Proxies const& proxies(this->view->get_proxies());
    for (std::size_t i = 0; i < proxies.size(); ++i)
        if (proxies[i].is_image)
            vec.push_back(proxies[i].name);
    std::sort(vec.begin(), vec.end());

    for (std::size_t i = 0; i < vec.size(); ++i)
    {
        std::string const& name(vec[i]);
        this->embeddings->addItem(QString(name.c_str()), (int)i);
        if (name == this->recent_embedding)
           this->embeddings->setCurrentIndex(this->embeddings->count() - 1);
    }
    this->embeddings->adjustSize();
    this->embeddings->setEnabled(!proxies.empty());
}

/* ---------------------------------------------------------------- */

void
ViewInspect::populate_exif_viewer (void)
{
    this->exif_viewer->setHtml("<i>No EXIF data available</i>");

    if (!this->view.get())
        return;

    mve::ByteImage::Ptr exif = this->view->get_data("exif");
    if (!exif.get())
        return;

    mve::image::ExifInfo info;
    try
    {
        info = mve::image::exif_extract(exif->get_byte_pointer(),
            exif->get_byte_size());
    }
    catch (std::exception& e)
    {
        std::cout << "Error while parsing EXIF embedding!" << std::endl;
        return;
    }

    std::stringstream ss;
    mve::image::exif_debug_print(ss, info);
    this->exif_viewer->setText(ss.str().c_str());
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_zoom_in (void)
{
    this->scroll_image->zoom_in();
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_zoom_out (void)
{
    this->scroll_image->zoom_out();
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_normal_size (void)
{
    this->scroll_image->reset_scale();
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_fit_to_window (void)
{
    bool fit = this->action_zoom_fit->isChecked();
    this->scroll_image->set_auto_scale(fit);
    this->update_actions();
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_open (void)
{
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Open File"), QDir::currentPath());

    if (filename.isEmpty())
        return;

    this->load_file(filename);
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_view_reload (void)
{
    if (!this->view.get())
        return;

    QMessageBox::StandardButton answer = QMessageBox::question(this,
        tr("Reload view?"), tr("Really reload view \"%1\" from file?"
        " Unsaved changes get lost, this cannot be undone.")
        .arg(this->view->get_filename().c_str()),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

    if (answer != QMessageBox::Yes)
        return;

    this->view->reload_mve_file();
    this->populate_embeddings();
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_details_toggled (void)
{
    bool show = this->action_show_details->isChecked();
    this->show_details(show);
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_embedding_selected (QString const& name)
{
    //std::cout << "Embedding selected: " << name.toStdString() << std::endl;
    this->set_embedding(name.toStdString());
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_image_clicked (int x, int y, QMouseEvent* event)
{
    if (event->buttons() & Qt::RightButton)
    {
        this->inspector->magnify(x, y);
        this->action_show_details->setChecked(true);
        this->show_details(true);
        this->image_details->setCurrentIndex(1);
    }
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_reload_embeddings (void)
{
    this->populate_embeddings();
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_tf_changed (TransferFunction func)
{
    this->func = func;
    this->inspector->set_transfer_function(this->func);

    if (this->image.get() == 0)
        return;

    if (this->image->get_type() != mve::IMAGE_TYPE_FLOAT)
        return;

/*
    std::cout << "Rebuilding image... Transfer function: "
        << "Zoom: " << func.zoom << ", Gamma: " << func.gamma
        << ", Clamp: [" << func.clamp_min << "," << func.clamp_max << "]"
        << ", Channels: " << this->func.red << ", "
        << this->func.green << ", " << this->func.blue
        << std::endl;
*/

    this->display_image((mve::FloatImage::Ptr)this->image);
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_ply_export (void)
{
    if (!this->view.get())
    {
        QMessageBox::information(this, tr("Export PLY"),
            tr("No view assigned!"));
        return;
    }

    /* Raise dialog that queries embedding names for:
     * depthmap, confidence map, normal map, color image.
     */
    PlyExportDialog d(this->view, this);
    int answer = d.exec();
    if (answer != QDialog::Accepted)
        return;

    QString filename = QFileDialog::getSaveFileName(this,
        "Export PLY file...");
    if (filename.size() == 0)
        return;

    /* Determine filename of PLY and XF file. */
    std::string plyname = filename.toStdString();
    if (util::string::right(plyname, 4) != ".ply")
        plyname += ".ply";
    std::string xfname = util::string::left
        (plyname, plyname.size() - 4) + ".xf";

    try
    {
        mve::geom::save_ply_view(this->view, plyname,
            d.depthmap, d.confidence, d.colorimage);
        mve::geom::save_xf_file(xfname, this->view->get_camera());
    }
    catch (std::exception& e)
    {
        QMessageBox::warning(this, tr("Export PLY"),
            tr("Error exporting PLY: %1").arg(QString(e.what())));
    }
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_image_export (void)
{
    if (!this->scroll_image->get_image()->pixmap())
    {
        QMessageBox::critical(this, "Cannot save image", "No such image");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this,
        "Export Image...");
    if (filename.size() == 0)
        return;

    try
    {
        this->scroll_image->save_image(filename.toStdString());
    }
    catch (std::exception& e)
    {
        QMessageBox::critical(this, "Cannot save image", e.what());
        return;
    }
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_copy_embedding (void)
{
    if (!this->image.get() || !this->view.get())
    {
        QMessageBox::warning(this, tr("Image Viewer"),
            tr("No embedding selected!"));
        return;
    }

    bool text_ok = false;
    QString qtext = QInputDialog::getText(this, tr("Copy Embedding"),
        tr("Enter a target name for the new embedding."),
        QLineEdit::Normal, this->recent_embedding.c_str(), &text_ok);

    if (!text_ok || qtext.isEmpty())
        return;

    std::string text = qtext.toStdString();

    if (text == this->recent_embedding)
    {
        QMessageBox::warning(this, tr("Image Viewer"),
            tr("Target and current embedding are the same!"));
        return;
    }

    bool exists = this->view->has_embedding(text);
    if (exists)
    {
        QMessageBox::StandardButton answer = QMessageBox::question(this,
            tr("Overwrite Embedding?"),
            tr("Target embedding exists. Overwrite?"),
            QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

        if (answer != QMessageBox::Yes)
            return;
    }

    mve::ImageBase::Ptr image_copy = this->image->duplicate();
    this->view->set_image(text, image_copy);
    this->populate_embeddings();
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_del_embedding (void)
{
    if (!this->image.get() || !this->view.get())
    {
        QMessageBox::warning(this, tr("Image Viewer"),
            tr("No embedding selected!"));
        return;
    }

    QMessageBox::StandardButton answer = QMessageBox::question(this,
        tr("Delete Embedding?"), tr("Really delete embedding \"%1\"?"
        " This cannot be undone.").arg(this->recent_embedding.c_str()),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

    if (answer != QMessageBox::Yes)
        return;

    this->view->remove_embedding(this->recent_embedding);
    this->load_recent_embedding();
    this->populate_embeddings();
}

/* ---------------------------------------------------------------- */

void
ViewInspect::on_save_view (void)
{
    if (!this->view.get())
    {
        QMessageBox::warning(this, tr("Image Viewer"),
            tr("No view selected!"));
        return;
    }

    try
    {
        this->view->save_mve_file();
    }
    catch (std::exception& e)
    {
        std::cout << "Error saving view: " << e.what() << std::endl;
        QMessageBox::critical(this, tr("Error saving view"),
            tr("Error saving view:\n%1").arg(e.what()));
    }
}

/* ---------------------------------------------------------------- */

void
ViewInspect::reset (void)
{
    this->view.reset();
    this->image.reset();
    this->inspector->reset();
    this->scroll_image->get_image()->setPixmap(QPixmap());
    this->embeddings->clear();
}
