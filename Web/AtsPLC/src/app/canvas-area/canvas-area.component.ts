import { Component, ViewChild, ElementRef, HostListener, AfterViewInit, OnDestroy, signal } from '@angular/core';
import { Subject, takeUntil } from 'rxjs';
import { HttpClientService } from '../services/http-client.service';
import { ForceRefreshService } from '../services/force-refresh.service';
import { DisplayData, DeviceConfig, ForceRefreshTarget, StoredImage } from '../main/models';

@Component({
	selector: 'app-canvas-area',
	standalone: true,
	imports: [],
	templateUrl: './canvas-area.component.html',
	styleUrls: ['./canvas-area.component.scss']
})
export class CanvasAreaComponent implements AfterViewInit, OnDestroy {
	@ViewChild('statusBarCanvas', { static: true }) statusBarCanvasRef!: ElementRef<HTMLCanvasElement>;
	@ViewChild('mainCanvas', { static: true }) mainCanvasRef!: ElementRef<HTMLCanvasElement>;
	@ViewChild('overlayCanvas', { static: true }) overlayCanvasRef!: ElementRef<HTMLCanvasElement>;
	@ViewChild('mainCanvasContainer', { static: true }) mainCanvasContainerRef!: ElementRef<HTMLDivElement>;

	currentScale = signal(1);

	private statusBarCtx!: CanvasRenderingContext2D;
	private mainCtx!: CanvasRenderingContext2D;
	private overlayCtx!: CanvasRenderingContext2D;
	private deviceConfig: DeviceConfig | null = null;
	private isFocused: boolean = false;
	private lastKeyDown = -1;
	private noChangeCount = 0;

	private destroy$ = new Subject<void>();
	private bitmapTimeoutId: any = null;

	private readonly CANVAS_STORAGE_KEY = 'mainCanvasBitmap';

	constructor(
		private httpClientService: HttpClientService,
		private forceRefreshService: ForceRefreshService
	) {}

	ngAfterViewInit(): void {
		this.httpClientService.getDeviceConfig()
			.pipe(takeUntil(this.destroy$))
			.subscribe({
				next: (config: DeviceConfig) => {
					this.httpClientService.setConcurrencyLimit(config.request_limit);
					this.deviceConfig = config;
					this.initializeCanvas(config);
					this.loadBitmap();
				},
				error: (err: any) => {
					console.error('Error getting device config:', err);
				}
			});
	}

	ngOnDestroy(): void {
		this.destroy$.next();
		this.destroy$.complete();

		if (this.bitmapTimeoutId) {
			clearTimeout(this.bitmapTimeoutId);
		}
	}

	private initializeCanvas(config: DeviceConfig): void {
		this.statusBarCanvasRef.nativeElement.width = config.display.width;
		this.statusBarCanvasRef.nativeElement.height = config.display.area_top;

		this.mainCanvasRef.nativeElement.width = config.display.width;
		this.mainCanvasRef.nativeElement.height = config.display.height - config.display.area_top;

		this.overlayCanvasRef.nativeElement.width = config.display.width;
		this.overlayCanvasRef.nativeElement.height = config.display.height - config.display.area_top;

		this.applyScale();

		this.statusBarCtx = this.statusBarCanvasRef.nativeElement.getContext('2d')!;
		this.mainCtx = this.mainCanvasRef.nativeElement.getContext('2d')!;
		this.overlayCtx = this.overlayCanvasRef.nativeElement.getContext('2d')!;

		this.restoreCanvasFromStorage();
		this.mainCanvasContainerRef.nativeElement.focus();
	}

	private loadBitmap(): void {
		this.httpClientService.getBitmap()
			.pipe(takeUntil(this.destroy$))
			.subscribe({
				next: (displayData: DisplayData) => {
					if (displayData.forceRefresh !== ForceRefreshTarget.None) {
						this.forceRefreshService.request(displayData.forceRefresh);
					}

					const retryIntervals = [20, 50, 100, 200, 300, 500, 500, 500, 700, 700, 1000, 1000, 1000, 1500, 1500, 2000];

					if (displayData.bitmap.byteLength == 0) {
						this.noChangeCount = Math.min(retryIntervals.length - 1, this.noChangeCount + 1);
						const interval = retryIntervals[this.noChangeCount];

						this.bitmapTimeoutId = setTimeout(() => {
							this.loadBitmap();
						}, interval);
						return;
					}

					this.noChangeCount = 0;
					this.drawSplitBitmap(displayData);

					this.bitmapTimeoutId = setTimeout(() => {
						this.loadBitmap();
					}, retryIntervals[0]);
				},
				error: (err: any) => {
					console.error('Error loading bitmap:', err);
					this.bitmapTimeoutId = setTimeout(() => {
						this.loadBitmap();
					}, 30000);
				}
			});
	}

	toggleScale(): void {
		this.currentScale.update(scale => scale === 1 ? 2 : 1);
		this.applyScale();
	}

	applyScale(): void {
		const scale = this.currentScale();
		this.setCanvasScale(this.statusBarCanvasRef.nativeElement, scale);
		this.setCanvasScale(this.mainCanvasRef.nativeElement, scale);
		this.setCanvasScale(this.overlayCanvasRef.nativeElement, scale);
	}

	private setCanvasScale(canvas: HTMLCanvasElement, scale: number): void {
		const originalWidth = canvas.width;
		const originalHeight = canvas.height;

		canvas.style.width = `${originalWidth * scale}px`;
		canvas.style.height = `${originalHeight * scale}px`;
		canvas.style.imageRendering = 'pixelated';
	}

	private resizeMainCanvas(offset: number, newHeight: number): void {
		if (!this.deviceConfig) {
			return;
		}

		const toIncrease = newHeight > this.mainCanvasRef.nativeElement.height;
		const topArea = this.deviceConfig.display.rails_height * offset;
		const bottomArea = this.mainCanvasRef.nativeElement.height - topArea
			- (this.deviceConfig.display.rails_height * (toIncrease ? 0 : 1));

		const imageDataBefore = topArea > 0
			? this.mainCtx.getImageData(0, 0, this.mainCanvasRef.nativeElement.width, topArea)
			: undefined;
		const imageDataAfter = bottomArea > 0
			? this.mainCtx.getImageData(0, topArea + (this.deviceConfig.display.rails_height * (toIncrease ? 0 : 1)),
				this.mainCanvasRef.nativeElement.width, bottomArea)
			: undefined;

		this.mainCanvasRef.nativeElement.height = newHeight;
		this.overlayCanvasRef.nativeElement.height = newHeight;

		if (imageDataBefore != undefined) {
			this.mainCtx.putImageData(imageDataBefore, 0, 0);
		}
		if (imageDataAfter != undefined) {
			this.mainCtx.putImageData(imageDataAfter, 0, this.deviceConfig.display.rails_height * (offset + (toIncrease ? 1 : 0)));
		}

		const scale = this.currentScale();
		this.setCanvasScale(this.mainCanvasRef.nativeElement, scale);
		this.setCanvasScale(this.overlayCanvasRef.nativeElement, scale);
	}

	private drawSplitBitmap(displayData: DisplayData): void {
		if (!this.deviceConfig || !this.mainCtx) {
			return;
		}

		const bitArray = new Uint8Array(displayData.bitmap);

		this.drawBitmapPartial(
			this.statusBarCtx,
			bitArray,
			0,
			this.deviceConfig.display.area_top,
			0,
			0,
			this.deviceConfig.display.width,
			this.deviceConfig.display.area_top
		);

		const height = Math.max(
			this.deviceConfig.display.height - this.deviceConfig.display.area_top,
			this.deviceConfig.display.rails_height * displayData.dataPaging.count
		);
		if (height != this.mainCanvasRef.nativeElement.height) {
			this.resizeMainCanvas(displayData.dataPaging.offset, height);
		}

		this.drawBitmapPartial(
			this.mainCtx,
			bitArray,
			this.deviceConfig.display.area_top,
			this.deviceConfig.display.height,
			0,
			this.deviceConfig.display.rails_height * displayData.dataPaging.offset,
			this.deviceConfig.display.area_width,
			Math.floor((this.deviceConfig.display.height - this.deviceConfig.display.area_top) / this.deviceConfig.display.rails_height) * this.deviceConfig.display.rails_height
		);

		this.drawActiveAreaRect(displayData);

		try {
			const storeImage: StoredImage = {
				height: this.mainCanvasRef.nativeElement.height,
				width: this.mainCanvasRef.nativeElement.width,
				dataUrl: this.mainCanvasRef.nativeElement.toDataURL()
			};
			localStorage.setItem(this.CANVAS_STORAGE_KEY, JSON.stringify(storeImage));
		} catch (e) {
			console.error('Error saving canvas to localStorage', e);
		}
	}

	private restoreCanvasFromStorage(): void {
		try {
			const dataUrl = localStorage.getItem(this.CANVAS_STORAGE_KEY);

			if (!dataUrl) {
				return;
			}
			const storeImage: StoredImage = JSON.parse(dataUrl);

			if (storeImage.width != this.mainCanvasRef.nativeElement.width) {
				console.warn('invalid localStorage data');
				return;
			}

			const img = new Image();
			img.onload = () => {
				this.resizeMainCanvas(0, storeImage.height);
				this.mainCtx.drawImage(img, 0, 0);
			};
			img.onerror = (err) => {
				console.error('Error loading image from localStorage data', err);
			};
			img.src = storeImage.dataUrl;
		} catch (e) {
			console.error('Error reading canvas data from localStorage', e);
		}
	}

	private drawActiveAreaRect(displayData: DisplayData): void {
		if (!this.deviceConfig) {
			return;
		}

		this.overlayCtx.clearRect(0, 0, this.overlayCanvasRef.nativeElement.width, this.overlayCanvasRef.nativeElement.height);

		const x = 0;
		const y = this.deviceConfig.display.rails_height * displayData.dataPaging.offset;
		const width = this.deviceConfig.display.width;
		const height = Math.floor((this.deviceConfig.display.height - this.deviceConfig.display.area_top) / this.deviceConfig.display.rails_height) * this.deviceConfig.display.rails_height;

		this.overlayCtx.globalAlpha = 0.2;
		this.overlayCtx.fillStyle = 'yellow';
		this.overlayCtx.fillRect(x, y, width, height);
		this.overlayCtx.strokeStyle = 'mediumblue';
		this.overlayCtx.lineWidth = 1;
		this.overlayCtx.strokeRect(x, y, width, height);
	}

	private drawBitmapPartial(
		ctx: CanvasRenderingContext2D,
		bitArray: Uint8Array,
		startY: number,
		endY: number,
		destX: number,
		destY: number,
		width: number,
		height: number
	): void {
		if (!this.deviceConfig) {
			return;
		}

		const imageData = ctx.createImageData(width, height);
		const rgbaData = imageData.data;

		switch (this.deviceConfig.display.id) {
			case 'ssd1306': {
				const scanLine = Math.ceil(this.deviceConfig.display.width / 8) * 8;
				for (let y = startY; y < endY; y++) {
					for (let x = 0; x < width; x++) {
						const page = Math.floor(y / 8);
						const byteIndex = page * scanLine + x;
						const bitIndex = y % 8;
						const rgbaIndex = ((y - startY) * width + x) * 4;

						rgbaData[rgbaIndex + 3] = 255;

						let bit;
						if (byteIndex >= bitArray.length) {
							bit = false;
						} else {
							bit = ((bitArray[byteIndex] >> bitIndex) & 1) != 0;
						}

						if (bit) {
							rgbaData[rgbaIndex] = 0;
							rgbaData[rgbaIndex + 1] = 0;
							rgbaData[rgbaIndex + 2] = 0;
						} else {
							rgbaData[rgbaIndex] = 255;
							rgbaData[rgbaIndex + 1] = 255;
							rgbaData[rgbaIndex + 2] = 255;
						}
					}
				}
				break;
			}

			case 'ssd1680': {
				const scanLine = Math.ceil(this.deviceConfig.display.height / 8);

				for (let y = startY; y < endY; y++) {
					for (let x = 0; x < width; x++) {
						const displayX = (this.deviceConfig.display.width - 1) - x;
						const page = Math.floor(y / 8);
						const byteIndex = page + displayX * scanLine;
						const bitIndex = y % 8;
						const rgbaIndex = ((y - startY) * width + x) * 4;

						rgbaData[rgbaIndex + 3] = 255;

						let bit;
						if (byteIndex >= bitArray.length) {
							bit = false;
						} else {
							bit = ((bitArray[byteIndex] >> (7 - bitIndex)) & 1) == 0;
						}

						if (bit) {
							rgbaData[rgbaIndex] = 0;
							rgbaData[rgbaIndex + 1] = 0;
							rgbaData[rgbaIndex + 2] = 0;
						} else {
							rgbaData[rgbaIndex] = 255;
							rgbaData[rgbaIndex + 1] = 255;
							rgbaData[rgbaIndex + 2] = 255;
						}
					}
				}
				break;
			}

			case 'linux-X11':
			default: {
				const scanLine = Math.ceil(this.deviceConfig.display.width / 8) * 8;
				for (let y = startY; y < endY; y++) {
					for (let x = 0; x < width; x++) {
						const pixelIndex = y * scanLine + x;
						const byteIndex = Math.floor(pixelIndex / 8);
						const bitIndex = pixelIndex % 8;
						const rgbaIndex = ((y - startY) * width + x) * 4;

						rgbaData[rgbaIndex + 3] = 255;
						let bit;

						if (byteIndex >= bitArray.length) {
							bit = false;
						} else {
							bit = ((bitArray[byteIndex] >> bitIndex) & 1) != 0;
						}

						if (bit) {
							rgbaData[rgbaIndex] = 0;
							rgbaData[rgbaIndex + 1] = 0;
							rgbaData[rgbaIndex + 2] = 0;
						} else {
							rgbaData[rgbaIndex] = 255;
							rgbaData[rgbaIndex + 1] = 255;
							rgbaData[rgbaIndex + 2] = 255;
						}
					}
				}
				break;
			}
		}

		ctx.putImageData(imageData, destX, destY);
	}

	private sendKeyPress(keyCode: number, down: number): void {
		this.httpClientService.postKeyPress({ key: keyCode, down }).subscribe();
	}

	onButtonDown(event: Event, keyCode: number): void {
		if (this.lastKeyDown == 1) {
			return;
		}
		this.lastKeyDown = 1;
		event.preventDefault();
		this.sendKeyPress(keyCode, 1);
	}

	onButtonUp(event: Event, keyCode: number): void {
		if (this.lastKeyDown == 0) {
			return;
		}
		this.lastKeyDown = 0;
		event.preventDefault();
		this.sendKeyPress(keyCode, 0);
	}

	private preventKeysDefaultHandling(event: KeyboardEvent): void {
		const handledKeys = ['ArrowUp', 'ArrowDown', 'ShiftRight'];

		if (handledKeys.includes(event.code)) {
			event.preventDefault();
		}
	}

	@HostListener('window:keydown', ['$event'])
	handleKeyDown(event: KeyboardEvent): void {
		if (!this.isFocused) {
			return;
		}

		this.preventKeysDefaultHandling(event);

		if (this.lastKeyDown == 1) {
			return;
		}
		this.lastKeyDown = 1;
		this.sendKeyPress(event.keyCode, 1);
	}

	@HostListener('window:keyup', ['$event'])
	handleKeyUp(event: KeyboardEvent): void {
		if (!this.isFocused) {
			return;
		}

		this.preventKeysDefaultHandling(event);

		if (this.lastKeyDown == 0) {
			return;
		}
		this.lastKeyDown = 0;
		this.sendKeyPress(event.keyCode, 0);
	}

	@HostListener('focusin', ['$event'])
	onFocusIn(event: FocusEvent): void {
		if (event.target === this.mainCanvasContainerRef.nativeElement) {
			this.isFocused = true;
		}
	}

	@HostListener('focusout', ['$event'])
	onFocusOut(event: FocusEvent): void {
		if (event.target === this.mainCanvasContainerRef.nativeElement) {
			this.isFocused = false;
		}
	}
}
