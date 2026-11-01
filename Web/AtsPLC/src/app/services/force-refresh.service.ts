import { Injectable } from '@angular/core';
import { Subject, Observable } from 'rxjs';

@Injectable({
	providedIn: 'root'
})
export class ForceRefreshService {
	private requests = new Subject<void>();
	private lastSeq: number | null = null;

	get requests$(): Observable<void> {
		return this.requests.asObservable();
	}

	notify(seq: number): void {
		if (this.lastSeq === seq) {
			return;
		}
		const seqKnown = this.lastSeq !== null;
		this.lastSeq = seq;

		if (!seqKnown) {
			return;
		}
		this.requests.next();
	}
}
