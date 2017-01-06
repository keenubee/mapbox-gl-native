package com.mapbox.mapboxsdk.maps;

import android.app.ActivityManager;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.PointF;
import android.graphics.RectF;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.DisplayMetrics;
import android.view.Surface;

import com.mapbox.mapboxsdk.annotations.Icon;
import com.mapbox.mapboxsdk.annotations.Marker;
import com.mapbox.mapboxsdk.annotations.Polygon;
import com.mapbox.mapboxsdk.annotations.Polyline;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.geometry.ProjectedMeters;
import com.mapbox.mapboxsdk.offline.OfflineManager;
import com.mapbox.mapboxsdk.style.layers.Layer;
import com.mapbox.mapboxsdk.style.layers.NoSuchLayerException;
import com.mapbox.mapboxsdk.style.sources.NoSuchSourceException;
import com.mapbox.mapboxsdk.style.sources.Source;
import com.mapbox.services.commons.geojson.Feature;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import timber.log.Timber;

// Class that wraps the native methods for convenience
final class NativeMapView {

  // Flag to indicating destroy was called
  private boolean destroyed = false;

  // Holds the pointer to JNI NativeMapView
  private long nativeMapViewPtr = 0;

  // Used for callbacks
  private MapView mapView;

  // Device density
  private final float pixelRatio;

  // Listeners for Map change events
  private CopyOnWriteArrayList<MapView.OnMapChangedListener> onMapChangedListeners;

  // Listener invoked to return a bitmap of the map
  private SnapshotRequest snapshotRequest;

  //
  // Static methods
  //

  static {
    System.loadLibrary("mapbox-gl");
  }

  //
  // Constructors
  //

  public NativeMapView(MapView mapView) {
    Context context = mapView.getContext();
    String dataPath = OfflineManager.getDatabasePath(context);

    // With the availability of offline, we're unifying the ambient (cache) and the offline
    // databases to be in the same folder, outside cache, to avoid automatic deletion from
    // the system
    String cachePath = dataPath;

    pixelRatio = context.getResources().getDisplayMetrics().density;
    String apkPath = context.getPackageCodePath();
    int availableProcessors = Runtime.getRuntime().availableProcessors();
    ActivityManager.MemoryInfo memoryInfo = new ActivityManager.MemoryInfo();
    ActivityManager activityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
    activityManager.getMemoryInfo(memoryInfo);
    long totalMemory = memoryInfo.availMem;
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
      totalMemory = memoryInfo.totalMem;
    }

    if (availableProcessors < 0) {
      throw new IllegalArgumentException("availableProcessors cannot be negative.");
    }

    if (totalMemory < 0) {
      throw new IllegalArgumentException("totalMemory cannot be negative.");
    }
    onMapChangedListeners = new CopyOnWriteArrayList<>();
    this.mapView = mapView;
    nativeMapViewPtr = nativeCreate(cachePath, dataPath, apkPath, pixelRatio, availableProcessors, totalMemory);
  }

  //
  // Methods
  //

  public void destroy() {
    nativeDestroy(nativeMapViewPtr);
    nativeMapViewPtr = 0;
    mapView = null;
    destroyed = true;
  }

  public void initializeDisplay() {
    if (destroyed) {
      return;
    }
    nativeInitializeDisplay(nativeMapViewPtr);
  }

  public void terminateDisplay() {
    if (destroyed) {
      return;
    }
    nativeTerminateDisplay(nativeMapViewPtr);
  }

  public void initializeContext() {
    if (destroyed) {
      return;
    }
    nativeInitializeContext(nativeMapViewPtr);
  }

  public void terminateContext() {
    if (destroyed) {
      return;
    }
    nativeTerminateContext(nativeMapViewPtr);
  }

  public void createSurface(Surface surface) {
    if (destroyed) {
      return;
    }
    nativeCreateSurface(nativeMapViewPtr, surface);
  }

  public void destroySurface() {
    if (destroyed) {
      return;
    }
    nativeDestroySurface(nativeMapViewPtr);
  }

  public void update() {
    if (destroyed) {
      return;
    }
    nativeUpdate(nativeMapViewPtr);
  }

  public void render() {
    if (destroyed) {
      return;
    }
    nativeRender(nativeMapViewPtr);
  }

  public void resizeView(int width, int height) {
    if (destroyed) {
      return;
    }
    width = (int) (width / pixelRatio);
    height = (int) (height / pixelRatio);

    if (width < 0) {
      throw new IllegalArgumentException("width cannot be negative.");
    }

    if (height < 0) {
      throw new IllegalArgumentException("height cannot be negative.");
    }

    if (width > 65535) {
      // we have seen edge cases where devices return incorrect values #6111
      Timber.e("Device returned an out of range width size, "
        + "capping value at 65535 instead of " + width);
      width = 65535;
    }

    if (height > 65535) {
      // we have seen edge cases where devices return incorrect values #6111
      Timber.e("Device returned an out of range height size, "
        + "capping value at 65535 instead of " + height);
      height = 65535;
    }
    nativeViewResize(nativeMapViewPtr, width, height);
  }

  public void resizeFramebuffer(int fbWidth, int fbHeight) {
    if (destroyed) {
      return;
    }
    if (fbWidth < 0) {
      throw new IllegalArgumentException("fbWidth cannot be negative.");
    }

    if (fbHeight < 0) {
      throw new IllegalArgumentException("fbHeight cannot be negative.");
    }

    if (fbWidth > 65535) {
      throw new IllegalArgumentException(
        "fbWidth cannot be greater than 65535.");
    }

    if (fbHeight > 65535) {
      throw new IllegalArgumentException(
        "fbHeight cannot be greater than 65535.");
    }
    nativeFramebufferResize(nativeMapViewPtr, fbWidth, fbHeight);
  }

  public void addClass(String clazz) {
    if (destroyed) {
      return;
    }
    nativeAddClass(nativeMapViewPtr, clazz);
  }

  public void removeClass(String clazz) {
    if (destroyed) {
      return;
    }
    nativeRemoveClass(nativeMapViewPtr, clazz);
  }

  public boolean hasClass(String clazz) {
    if (destroyed) {
      return false;
    }
    return nativeHasClass(nativeMapViewPtr, clazz);
  }

  public void setClasses(List<String> classes) {
    if (destroyed) {
      return;
    }
    nativeSetClasses(nativeMapViewPtr, classes);
  }

  public List<String> getClasses() {
    if (destroyed) {
      return new ArrayList<>();
    }
    return nativeGetClasses(nativeMapViewPtr);
  }

  public void setStyleUrl(String url) {
    if (destroyed) {
      return;
    }
    nativeSetStyleUrl(nativeMapViewPtr, url);
  }

  public String getStyleUrl() {
    if (destroyed) {
      return null;
    }
    return nativeGetStyleUrl(nativeMapViewPtr);
  }

  public void setStyleJson(String newStyleJson) {
    if (destroyed) {
      return;
    }
    nativeSetStyleJson(nativeMapViewPtr, newStyleJson);
  }

  public String getStyleJson() {
    if (destroyed) {
      return null;
    }
    return nativeGetStyleJson(nativeMapViewPtr);
  }

  public void setAccessToken(String accessToken) {
    if (destroyed) {
      return;
    }
    nativeSetAccessToken(nativeMapViewPtr, accessToken);
  }

  public String getAccessToken() {
    if (destroyed) {
      return null;
    }
    return nativeGetAccessToken(nativeMapViewPtr);
  }

  public void cancelTransitions() {
    if (destroyed) {
      return;
    }
    nativeCancelTransitions(nativeMapViewPtr);
  }

  public void setGestureInProgress(boolean inProgress) {
    if (destroyed) {
      return;
    }
    nativeSetGestureInProgress(nativeMapViewPtr, inProgress);
  }

  public void moveBy(double dx, double dy) {
    if (destroyed) {
      return;
    }
    moveBy(dx, dy, 0);
  }

  public void moveBy(double dx, double dy, long duration) {
    if (destroyed) {
      return;
    }
    nativeMoveBy(nativeMapViewPtr, dx / pixelRatio, dy / pixelRatio, duration);
  }

  public void setLatLng(LatLng latLng) {
    if (destroyed) {
      return;
    }
    setLatLng(latLng, 0);
  }

  public void setLatLng(LatLng latLng, long duration) {
    if (destroyed) {
      return;
    }
    nativeSetLatLng(nativeMapViewPtr, latLng.getLatitude(), latLng.getLongitude(), duration);
  }

  public LatLng getLatLng() {
    if (destroyed) {
      return new LatLng();
    }
    return nativeGetLatLng(nativeMapViewPtr);
  }

  public void resetPosition() {
    if (destroyed) {
      return;
    }
    nativeResetPosition(nativeMapViewPtr);
  }

  public double getPitch() {
    if (destroyed) {
      return 0;
    }
    return nativeGetPitch(nativeMapViewPtr);
  }

  public void setPitch(double pitch, long duration) {
    if (destroyed) {
      return;
    }
    nativeSetPitch(nativeMapViewPtr, pitch, duration);
  }

  public void scaleBy(double ds) {
    if (destroyed) {
      return;
    }
    scaleBy(ds, Double.NaN, Double.NaN);
  }

  public void scaleBy(double ds, double cx, double cy) {
    if (destroyed) {
      return;
    }
    scaleBy(ds, cx, cy, 0);
  }

  public void scaleBy(double ds, double cx, double cy, long duration) {
    if (destroyed) {
      return;
    }
    nativeScaleBy(nativeMapViewPtr, ds, cx / pixelRatio, cy / pixelRatio, duration);
  }

  public void setScale(double scale) {
    if (destroyed) {
      return;
    }
    setScale(scale, Double.NaN, Double.NaN);
  }

  public void setScale(double scale, double cx, double cy) {
    if (destroyed) {
      return;
    }
    setScale(scale, cx, cy, 0);
  }

  public void setScale(double scale, double cx, double cy, long duration) {
    if (destroyed) {
      return;
    }
    nativeSetScale(nativeMapViewPtr, scale, cx / pixelRatio, cy / pixelRatio, duration);
  }

  public double getScale() {
    if (destroyed) {
      return 0;
    }
    return nativeGetScale(nativeMapViewPtr);
  }

  public void setZoom(double zoom) {
    if (destroyed) {
      return;
    }
    setZoom(zoom, 0);
  }

  public void setZoom(double zoom, long duration) {
    if (destroyed) {
      return;
    }
    nativeSetZoom(nativeMapViewPtr, zoom, duration);
  }

  public double getZoom() {
    if (destroyed) {
      return 0;
    }
    return nativeGetZoom(nativeMapViewPtr);
  }

  public void resetZoom() {
    if (destroyed) {
      return;
    }
    nativeResetZoom(nativeMapViewPtr);
  }

  public void setMinZoom(double zoom) {
    if (destroyed) {
      return;
    }
    nativeSetMinZoom(nativeMapViewPtr, zoom);
  }

  public double getMinZoom() {
    if (destroyed) {
      return 0;
    }
    return nativeGetMinZoom(nativeMapViewPtr);
  }

  public void setMaxZoom(double zoom) {
    if (destroyed) {
      return;
    }
    nativeSetMaxZoom(nativeMapViewPtr, zoom);
  }

  public double getMaxZoom() {
    if (destroyed) {
      return 0;
    }
    return nativeGetMaxZoom(nativeMapViewPtr);
  }

  public void rotateBy(double sx, double sy, double ex, double ey) {
    if (destroyed) {
      return;
    }
    rotateBy(sx, sy, ex, ey, 0);
  }

  public void rotateBy(double sx, double sy, double ex, double ey,
                       long duration) {
    if (destroyed) {
      return;
    }
    nativeRotateBy(nativeMapViewPtr, sx / pixelRatio, sy / pixelRatio, ex, ey, duration);
  }

  public void setContentPadding(int[] padding) {
    if (destroyed) {
      return;
    }
    nativeSetContentPadding(nativeMapViewPtr,
      padding[1] / pixelRatio,
      padding[0] / pixelRatio,
      padding[3] / pixelRatio,
      padding[2] / pixelRatio);
  }

  public void setBearing(double degrees) {
    if (destroyed) {
      return;
    }
    setBearing(degrees, 0);
  }

  public void setBearing(double degrees, long duration) {
    if (destroyed) {
      return;
    }
    nativeSetBearing(nativeMapViewPtr, degrees, duration);
  }

  public void setBearing(double degrees, double cx, double cy) {
    if (destroyed) {
      return;
    }
    nativeSetBearingXY(nativeMapViewPtr, degrees, cx / pixelRatio, cy / pixelRatio);
  }

  public double getBearing() {
    if (destroyed) {
      return 0;
    }
    return nativeGetBearing(nativeMapViewPtr);
  }

  public void resetNorth() {
    if (destroyed) {
      return;
    }
    nativeResetNorth(nativeMapViewPtr);
  }

  public long addMarker(Marker marker) {
    if (destroyed) {
      return 0;
    }
    Marker[] markers = {marker};
    return nativeAddMarkers(nativeMapViewPtr, markers)[0];
  }

  public long[] addMarkers(List<Marker> markers) {
    if (destroyed) {
      return new long[] {};
    }
    return nativeAddMarkers(nativeMapViewPtr, markers.toArray(new Marker[markers.size()]));
  }

  public long addPolyline(Polyline polyline) {
    if (destroyed) {
      return 0;
    }
    Polyline[] polylines = {polyline};
    return nativeAddPolylines(nativeMapViewPtr, polylines)[0];
  }

  public long[] addPolylines(List<Polyline> polylines) {
    if (destroyed) {
      return new long[] {};
    }
    return nativeAddPolylines(nativeMapViewPtr, polylines.toArray(new Polyline[polylines.size()]));
  }

  public long addPolygon(Polygon polygon) {
    if (destroyed) {
      return 0;
    }
    Polygon[] polygons = {polygon};
    return nativeAddPolygons(nativeMapViewPtr, polygons)[0];
  }

  public long[] addPolygons(List<Polygon> polygons) {
    if (destroyed) {
      return new long[] {};
    }
    return nativeAddPolygons(nativeMapViewPtr, polygons.toArray(new Polygon[polygons.size()]));
  }

  public void updateMarker(Marker marker) {
    if (destroyed) {
      return;
    }
    LatLng position = marker.getPosition();
    Icon icon = marker.getIcon();
    nativeUpdateMarker(nativeMapViewPtr, marker.getId(), position.getLatitude(), position.getLongitude(), icon.getId());
  }

  public void updatePolygon(Polygon polygon) {
    if (destroyed) {
      return;
    }
    nativeUpdatePolygon(nativeMapViewPtr, polygon.getId(), polygon);
  }

  public void updatePolyline(Polyline polyline) {
    if (destroyed) {
      return;
    }
    nativeUpdatePolyline(nativeMapViewPtr, polyline.getId(), polyline);
  }

  public void removeAnnotation(long id) {
    if (destroyed) {
      return;
    }
    long[] ids = {id};
    removeAnnotations(ids);
  }

  public void removeAnnotations(long[] ids) {
    if (destroyed) {
      return;
    }
    nativeRemoveAnnotations(nativeMapViewPtr, ids);
  }

  public long[] queryPointAnnotations(RectF rect) {
    if (destroyed) {
      return new long[] {};
    }
    return nativeQueryPointAnnotations(nativeMapViewPtr, rect);
  }

  public void addAnnotationIcon(String symbol, int width, int height, float scale, byte[] pixels) {
    if (destroyed) {
      return;
    }
    nativeAddAnnotationIcon(nativeMapViewPtr, symbol, width, height, scale, pixels);
  }

  public void setVisibleCoordinateBounds(LatLng[] coordinates, RectF padding, double direction, long duration) {
    if (destroyed) {
      return;
    }
    nativeSetVisibleCoordinateBounds(nativeMapViewPtr, coordinates, padding, direction, duration);
  }

  public void onLowMemory() {
    if (destroyed) {
      return;
    }
    nativeOnLowMemory(nativeMapViewPtr);
  }

  public void setDebug(boolean debug) {
    if (destroyed) {
      return;
    }
    nativeSetDebug(nativeMapViewPtr, debug);
  }

  public void cycleDebugOptions() {
    if (destroyed) {
      return;
    }
    nativeToggleDebug(nativeMapViewPtr);
  }

  public boolean getDebug() {
    if (destroyed) {
      return false;
    }
    return nativeGetDebug(nativeMapViewPtr);
  }

  public boolean isFullyLoaded() {
    if (destroyed) {
      return false;
    }
    return nativeIsFullyLoaded(nativeMapViewPtr);
  }

  public void setReachability(boolean status) {
    if (destroyed) {
      return;
    }
    nativeSetReachability(nativeMapViewPtr, status);
  }

  public double getMetersPerPixelAtLatitude(double lat) {
    if (destroyed) {
      return 0;
    }
    return nativeGetMetersPerPixelAtLatitude(nativeMapViewPtr, lat, getZoom());
  }

  public ProjectedMeters projectedMetersForLatLng(LatLng latLng) {
    if (destroyed) {
      return null;
    }
    return nativeProjectedMetersForLatLng(nativeMapViewPtr, latLng.getLatitude(), latLng.getLongitude());
  }

  public LatLng latLngForProjectedMeters(ProjectedMeters projectedMeters) {
    if (destroyed) {
      return new LatLng();
    }
    return nativeLatLngForProjectedMeters(nativeMapViewPtr, projectedMeters.getNorthing(),
      projectedMeters.getEasting());
  }

  public PointF pixelForLatLng(LatLng latLng) {
    if (destroyed) {
      return new PointF();
    }
    PointF pointF = nativePixelForLatLng(nativeMapViewPtr, latLng.getLatitude(), latLng.getLongitude());
    pointF.set(pointF.x * pixelRatio, pointF.y * pixelRatio);
    return pointF;
  }

  public LatLng latLngForPixel(PointF pixel) {
    if (destroyed) {
      return new LatLng();
    }
    return nativeLatLngForPixel(nativeMapViewPtr, pixel.x / pixelRatio, pixel.y / pixelRatio);
  }

  public double getTopOffsetPixelsForAnnotationSymbol(String symbolName) {
    if (destroyed) {
      return 0;
    }
    return nativeGetTopOffsetPixelsForAnnotationSymbol(nativeMapViewPtr, symbolName);
  }

  public void jumpTo(double angle, LatLng center, double pitch, double zoom) {
    if (destroyed) {
      return;
    }
    nativeJumpTo(nativeMapViewPtr, angle, center.getLatitude(), center.getLongitude(), pitch, zoom);
  }

  public void easeTo(double angle, LatLng center, long duration, double pitch, double zoom,
                     boolean easingInterpolator) {
    if (destroyed) {
      return;
    }
    nativeEaseTo(nativeMapViewPtr, angle, center.getLatitude(), center.getLongitude(), duration, pitch, zoom,
      easingInterpolator);
  }

  public void flyTo(double angle, LatLng center, long duration, double pitch, double zoom) {
    if (destroyed) {
      return;
    }
    nativeFlyTo(nativeMapViewPtr, angle, center.getLatitude(), center.getLongitude(), duration, pitch, zoom);
  }

  public double[] getCameraValues() {
    if (destroyed) {
      return new double[] {};
    }
    return nativeGetCameraValues(nativeMapViewPtr);
  }

  // Runtime style Api

  public Layer getLayer(String layerId) {
    if (destroyed) {
      return null;
    }
    return nativeGetLayer(nativeMapViewPtr, layerId);
  }

  public void addLayer(@NonNull Layer layer, @Nullable String before) {
    if (destroyed) {
      return;
    }
    nativeAddLayer(nativeMapViewPtr, layer.getNativePtr(), before);
  }

  public void removeLayer(@NonNull String layerId) throws NoSuchLayerException {
    if (destroyed) {
      return;
    }
    nativeRemoveLayerById(nativeMapViewPtr, layerId);
  }

  public void removeLayer(@NonNull Layer layer) throws NoSuchLayerException {
    if (destroyed) {
      return;
    }
    nativeRemoveLayer(nativeMapViewPtr, layer.getNativePtr());
  }

  public Source getSource(@NonNull String sourceId) {
    if (destroyed) {
      return null;
    }
    return nativeGetSource(nativeMapViewPtr, sourceId);
  }

  public void addSource(@NonNull Source source) {
    if (destroyed) {
      return;
    }
    nativeAddSource(nativeMapViewPtr, source.getNativePtr());
  }

  public void removeSource(@NonNull String sourceId) throws NoSuchSourceException {
    if (destroyed) {
      return;
    }
    nativeRemoveSourceById(nativeMapViewPtr, sourceId);
  }

  public void removeSource(@NonNull Source source) throws NoSuchSourceException {
    if (destroyed) {
      return;
    }
    nativeRemoveSource(nativeMapViewPtr, source.getNativePtr());
  }

  public void addImage(@NonNull String name, @NonNull Bitmap image) {
    if (destroyed) {
      return;
    }
    //Check/correct config
    if (image.getConfig() != Bitmap.Config.ARGB_8888) {
      image = image.copy(Bitmap.Config.ARGB_8888, false);
    }

    //Get pixels
    ByteBuffer buffer = ByteBuffer.allocate(image.getByteCount());
    image.copyPixelsToBuffer(buffer);

    //Determine pixel ratio
    float density = image.getDensity() == Bitmap.DENSITY_NONE ? Bitmap.DENSITY_NONE : image.getDensity();
    float pixelRatio = density / DisplayMetrics.DENSITY_DEFAULT;

    nativeAddImage(nativeMapViewPtr, name, image.getWidth(), image.getHeight(), pixelRatio, buffer.array());
  }

  public void removeImage(String name) {
    if (destroyed) {
      return;
    }
    nativeRemoveImage(nativeMapViewPtr, name);
  }

  // Feature querying

  @NonNull
  public List<Feature> queryRenderedFeatures(PointF coordinates, String... layerIds) {
    if (destroyed) {
      return new ArrayList<>();
    }
    Feature[] features = nativeQueryRenderedFeaturesForPoint(nativeMapViewPtr, coordinates.x / pixelRatio,
      coordinates.y / pixelRatio, layerIds);
    return features != null ? Arrays.asList(features) : new ArrayList<Feature>();
  }

  @NonNull
  public List<Feature> queryRenderedFeatures(RectF coordinates, String... layerIds) {
    if (destroyed) {
      return new ArrayList<>();
    }
    Feature[] features = nativeQueryRenderedFeaturesForBox(
      nativeMapViewPtr,
      coordinates.left / pixelRatio,
      coordinates.top / pixelRatio,
      coordinates.right / pixelRatio,
      coordinates.bottom / pixelRatio,
      layerIds);
    return features != null ? Arrays.asList(features) : new ArrayList<Feature>();
  }

  public void scheduleTakeSnapshot() {
    if (destroyed) {
      return;
    }
    nativeScheduleTakeSnapshot(nativeMapViewPtr);
  }

  public void setApiBaseUrl(String baseUrl) {
    if (destroyed) {
      return;
    }
    nativeSetAPIBaseURL(nativeMapViewPtr, baseUrl);
  }

  public float getPixelRatio() {
    if (destroyed) {
      return 0;
    }
    return pixelRatio;
  }

  public Context getContext() {
    return mapView.getContext();
  }

  //
  // Callbacks
  //

  protected void onInvalidate() {
    mapView.onInvalidate();
  }

  protected void onMapChanged(int rawChange) {
    if (onMapChangedListeners != null) {
      for (MapView.OnMapChangedListener onMapChangedListener : onMapChangedListeners) {
        onMapChangedListener.onMapChanged(rawChange);
      }
    }
  }

  protected void onFpsChanged(double fps) {
    mapView.onFpsChanged(fps);
  }

  protected void onSnapshotReady(byte[] bytes) {
    if (snapshotRequest != null && bytes != null) {
      BitmapFactory.Options options = new BitmapFactory.Options();
      options.inBitmap = snapshotRequest.getBitmap();  // the old Bitmap to be reused
      options.inMutable = true;
      options.inSampleSize = 1;
      Bitmap bitmap = BitmapFactory.decodeByteArray(bytes, 0, bytes.length, options);

      MapboxMap.SnapshotReadyCallback callback = snapshotRequest.getCallback();
      if (callback != null) {
        callback.onSnapshotReady(bitmap);
      }
    }
  }

  //
  // JNI methods
  //

  private native long nativeCreate(String cachePath, String dataPath, String apkPath, float pixelRatio,
                                   int availableProcessors, long totalMemory);

  private native void nativeDestroy(long nativeMapViewPtr);

  private native void nativeInitializeDisplay(long nativeMapViewPtr);

  private native void nativeTerminateDisplay(long nativeMapViewPtr);

  private native void nativeInitializeContext(long nativeMapViewPtr);

  private native void nativeTerminateContext(long nativeMapViewPtr);

  private native void nativeCreateSurface(long nativeMapViewPtr,
                                          Surface surface);

  private native void nativeDestroySurface(long nativeMapViewPtr);

  private native void nativeUpdate(long nativeMapViewPtr);

  private native void nativeRender(long nativeMapViewPtr);

  private native void nativeViewResize(long nativeMapViewPtr, int width, int height);

  private native void nativeFramebufferResize(long nativeMapViewPtr, int fbWidth, int fbHeight);

  private native void nativeAddClass(long nativeMapViewPtr, String clazz);

  private native void nativeRemoveClass(long nativeMapViewPtr, String clazz);

  private native boolean nativeHasClass(long nativeMapViewPtr, String clazz);

  private native void nativeSetClasses(long nativeMapViewPtr,
                                       List<String> classes);

  private native List<String> nativeGetClasses(long nativeMapViewPtr);

  private native void nativeSetStyleUrl(long nativeMapViewPtr, String url);

  private native String nativeGetStyleUrl(long nativeMapViewPtr);

  private native void nativeSetStyleJson(long nativeMapViewPtr, String newStyleJson);

  private native String nativeGetStyleJson(long nativeMapViewPtr);

  private native void nativeSetAccessToken(long nativeMapViewPtr, String accessToken);

  private native String nativeGetAccessToken(long nativeMapViewPtr);

  private native void nativeCancelTransitions(long nativeMapViewPtr);

  private native void nativeSetGestureInProgress(long nativeMapViewPtr, boolean inProgress);

  private native void nativeMoveBy(long nativeMapViewPtr, double dx,
                                   double dy, long duration);

  private native void nativeSetLatLng(long nativeMapViewPtr, double latitude, double longitude,
                                      long duration);

  private native LatLng nativeGetLatLng(long nativeMapViewPtr);

  private native void nativeResetPosition(long nativeMapViewPtr);

  private native double nativeGetPitch(long nativeMapViewPtr);

  private native void nativeSetPitch(long nativeMapViewPtr, double pitch, long duration);

  private native void nativeScaleBy(long nativeMapViewPtr, double ds,
                                    double cx, double cy, long duration);

  private native void nativeSetScale(long nativeMapViewPtr, double scale,
                                     double cx, double cy, long duration);

  private native double nativeGetScale(long nativeMapViewPtr);

  private native void nativeSetZoom(long nativeMapViewPtr, double zoom,
                                    long duration);

  private native double nativeGetZoom(long nativeMapViewPtr);

  private native void nativeResetZoom(long nativeMapViewPtr);

  private native void nativeSetMinZoom(long nativeMapViewPtr, double zoom);

  private native double nativeGetMinZoom(long nativeMapViewPtr);

  private native void nativeSetMaxZoom(long nativeMapViewPtr, double zoom);

  private native double nativeGetMaxZoom(long nativeMapViewPtr);

  private native void nativeRotateBy(long nativeMapViewPtr, double sx,
                                     double sy, double ex, double ey, long duration);

  private native void nativeSetContentPadding(long nativeMapViewPtr, double top, double left, double bottom,
                                              double right);

  private native void nativeSetBearing(long nativeMapViewPtr, double degrees,
                                       long duration);

  private native void nativeSetBearingXY(long nativeMapViewPtr, double degrees,
                                         double cx, double cy);

  private native double nativeGetBearing(long nativeMapViewPtr);

  private native void nativeResetNorth(long nativeMapViewPtr);

  private native void nativeUpdateMarker(long nativeMapViewPtr, long markerId, double lat, double lon, String iconId);

  private native long[] nativeAddMarkers(long nativeMapViewPtr, Marker[] markers);

  private native long[] nativeAddPolylines(long nativeMapViewPtr, Polyline[] polylines);

  private native long[] nativeAddPolygons(long nativeMapViewPtr, Polygon[] polygons);

  private native void nativeRemoveAnnotations(long nativeMapViewPtr, long[] id);

  private native long[] nativeQueryPointAnnotations(long nativeMapViewPtr, RectF rect);

  private native void nativeAddAnnotationIcon(long nativeMapViewPtr, String symbol,
                                              int width, int height, float scale, byte[] pixels);

  private native void nativeSetVisibleCoordinateBounds(long nativeMapViewPtr, LatLng[] coordinates,
                                                       RectF padding, double direction, long duration);

  private native void nativeOnLowMemory(long nativeMapViewPtr);

  private native void nativeSetDebug(long nativeMapViewPtr, boolean debug);

  private native void nativeToggleDebug(long nativeMapViewPtr);

  private native boolean nativeGetDebug(long nativeMapViewPtr);

  private native boolean nativeIsFullyLoaded(long nativeMapViewPtr);

  private native void nativeSetReachability(long nativeMapViewPtr, boolean status);

  private native double nativeGetMetersPerPixelAtLatitude(long nativeMapViewPtr, double lat, double zoom);

  private native ProjectedMeters nativeProjectedMetersForLatLng(long nativeMapViewPtr, double latitude,
                                                                double longitude);

  private native LatLng nativeLatLngForProjectedMeters(long nativeMapViewPtr, double northing, double easting);

  private native PointF nativePixelForLatLng(long nativeMapViewPtr, double lat, double lon);

  private native LatLng nativeLatLngForPixel(long nativeMapViewPtr, float x, float y);

  private native double nativeGetTopOffsetPixelsForAnnotationSymbol(long nativeMapViewPtr, String symbolName);

  private native void nativeJumpTo(long nativeMapViewPtr, double angle, double latitude, double longitude,
                                   double pitch, double zoom);

  private native void nativeEaseTo(long nativeMapViewPtr, double angle, double latitude, double longitude,
                                   long duration, double pitch, double zoom, boolean easingInterpolator);

  private native void nativeFlyTo(long nativeMapViewPtr, double angle, double latitude, double longitude,
                                  long duration, double pitch, double zoom);

  private native double[] nativeGetCameraValues(long nativeMapViewPtr);

  private native Layer nativeGetLayer(long nativeMapViewPtr, String layerId);

  private native void nativeAddLayer(long nativeMapViewPtr, long layerPtr, String before);

  private native void nativeRemoveLayerById(long nativeMapViewPtr, String layerId) throws NoSuchLayerException;

  private native void nativeRemoveLayer(long nativeMapViewPtr, long layerId) throws NoSuchLayerException;

  private native Source nativeGetSource(long nativeMapViewPtr, String sourceId);

  private native void nativeAddSource(long nativeMapViewPtr, long nativeSourcePtr);

  private native void nativeRemoveSourceById(long nativeMapViewPtr, String sourceId) throws NoSuchSourceException;

  private native void nativeRemoveSource(long nativeMapViewPtr, long sourcePtr) throws NoSuchSourceException;

  private native void nativeAddImage(long nativeMapViewPtr, String name, int width, int height, float pixelRatio,
                                     byte[] array);

  private native void nativeRemoveImage(long nativeMapViewPtr, String name);

  private native void nativeUpdatePolygon(long nativeMapViewPtr, long polygonId, Polygon polygon);

  private native void nativeUpdatePolyline(long nativeMapviewPtr, long polylineId, Polyline polyline);

  private native void nativeScheduleTakeSnapshot(long nativeMapViewPtr);

  private native Feature[] nativeQueryRenderedFeaturesForPoint(long nativeMapViewPtr, float x, float y, String[]
    layerIds);

  private native Feature[] nativeQueryRenderedFeaturesForBox(long nativeMapViewPtr, float left, float top, float right,
                                                             float bottom, String[] layerIds);

  private native void nativeSetAPIBaseURL(long nativeMapViewPtr, String baseUrl);

  int getWidth() {
    if (destroyed) {
      return 0;
    }
    return mapView.getWidth();
  }

  int getHeight() {
    if (destroyed) {
      return 0;
    }
    return mapView.getHeight();
  }

  //
  // MapChangeEvents
  //

  void addOnMapChangedListener(@NonNull MapView.OnMapChangedListener listener) {
    onMapChangedListeners.add(listener);
  }

  void removeOnMapChangedListener(@NonNull MapView.OnMapChangedListener listener) {
    onMapChangedListeners.remove(listener);
  }

  //
  // Snapshot
  //

  void addSnapshotCallback(@NonNull MapboxMap.SnapshotReadyCallback callback, @Nullable Bitmap bitmap) {
    snapshotRequest = new SnapshotRequest(bitmap, callback);
    scheduleTakeSnapshot();
    render();
  }

  private static class SnapshotRequest {
    private Bitmap bitmap;
    private MapboxMap.SnapshotReadyCallback callback;

    SnapshotRequest(Bitmap bitmap, MapboxMap.SnapshotReadyCallback callback) {
      this.bitmap = bitmap;
      this.callback = callback;
    }

    public Bitmap getBitmap() {
      return bitmap;
    }

    public MapboxMap.SnapshotReadyCallback getCallback() {
      return callback;
    }
  }
}
